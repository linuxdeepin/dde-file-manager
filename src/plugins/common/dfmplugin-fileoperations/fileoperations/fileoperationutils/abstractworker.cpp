// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractworker.h"
#include "workerdata.h"
#include "errormessageandaction.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QWaitCondition>
#include <QMutex>
#include <QApplication>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QDebug>
#include <qplatformdefs.h>

DPFILEOPERATIONS_USE_NAMESPACE

std::atomic_bool AbstractWorker::bigFileCopy { false };

/*!
 * \brief setWorkArgs 设置当前任务的参数
 * \param args 参数
 */
void AbstractWorker::setWorkArgs(const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target,
                                 const AbstractJobHandler::JobFlags &flags)
{
    if (!handle) {
        fmWarning() << "JobHandlePointer is a nullptr, setWorkArgs failed!";
        return;
    }
    connect(this, &AbstractWorker::startWork, this, &AbstractWorker::doWork);
    workData.reset(new WorkerData);
    workData->dirSize = FileUtils::getMemoryPageSize();
    this->handle = handle;
    initHandleConnects(handle);
    this->sourceUrls = sources;
    this->targetUrl = target;
    targetOrgUrl = targetUrl;
    isConvert = flags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kRevocation);
    workData->jobFlags = flags;
}

/*!
 * \brief doOperateWork 处理用户的操作 不在拷贝线程执行的函数，协同类直接调用
 * \param actions 当前操作
 */
void AbstractWorker::doOperateWork(AbstractJobHandler::SupportActions actions, AbstractJobHandler::JobErrorType error, const quint64 id)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)) {
        Q_EMIT removeTaskWidget();
        return stopAllThread();
    }
    if (actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction))
        return pauseAllThread();
    if (actions.testFlag(AbstractJobHandler::SupportAction::kResumAction))
        return resumeAllThread();

    getAction(actions);

    if (actions.testFlag(AbstractJobHandler::SupportAction::kRememberAction) && error != AbstractJobHandler::JobErrorType::kNoError)
        workData->errorOfAction.insert(error, currentAction);

    // dealing error thread
    if (workData->signalThread) {
        if (copyOtherFileWorker)
            copyOtherFileWorker->operateAction(currentAction);
        resume();
        return;
    }

    if (id == quintptr(this)) {
        return resume();
    }

    for (auto worker : threadCopyWorker) {
        if (id == quintptr(worker.data())) {
            worker->operateAction(currentAction);
            return;
        }
    }
}

/*!
 * \brief AbstractWorker::stop stop task
 */
void AbstractWorker::stop()
{
    setStat(AbstractJobHandler::JobState::kStopState);
    if (statisticsFilesSizeJob)
        statisticsFilesSizeJob->stop();

    if (updateProgressTimer)
        updateProgressTimer->stopTimer();

    if (updateProgressThread) {
        updateProgressThread->quit();
        updateProgressThread->wait();
    }
    waitCondition.wakeAll();
}
/*!
 * \brief AbstractWorker::pause paused task
 */
void AbstractWorker::pause()
{
    if (currentState == AbstractJobHandler::JobState::kPauseState)
        return;
    if (speedtimer) {
        elapsed += speedtimer->elapsed();
        speedtimerList.append(speedtimer);
        speedtimer = nullptr;
        JobInfoPointer info(new QMap<quint8, QVariant>);
        info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
        info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));
        info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, QVariant::fromValue(0));
        info->insert(AbstractJobHandler::NotifyInfoKey::kRemindTimeKey, QVariant::fromValue(-1));

        emit stateChangedNotify(info);
        emit speedUpdatedNotify(info);
    }

    setStat(AbstractJobHandler::JobState::kPauseState);
}
/*!
 * \brief AbstractWorker::resume resume task
 */
void AbstractWorker::resume()
{
    setStat(AbstractJobHandler::JobState::kRunningState);
    if (!speedtimer) {
        speedtimer = new QElapsedTimer;
        speedtimer->start();
    }

    waitCondition.wakeAll();
}

void AbstractWorker::getAction(AbstractJobHandler::SupportActions actions)
{
    retry = false;
    if (actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
        currentAction = AbstractJobHandler::SupportAction::kCancelAction;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kCoexistAction)) {
        currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kSkipAction)) {
        currentAction = AbstractJobHandler::SupportAction::kSkipAction;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kMergeAction)) {
        currentAction = AbstractJobHandler::SupportAction::kMergeAction;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kReplaceAction)) {
        currentAction = AbstractJobHandler::SupportAction::kReplaceAction;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction)) {
        currentAction = AbstractJobHandler::SupportAction::kRetryAction;
        retry = workData->signalThread ? false : true;
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kEnforceAction)) {
        currentAction = AbstractJobHandler::SupportAction::kEnforceAction;
    } else {
        currentAction = AbstractJobHandler::SupportAction::kNoAction;
    }
}

QUrl AbstractWorker::parentUrl(const QUrl &url)
{
    return FileOperationsUtils::parentUrl(url);
}

FileInfo::FileType AbstractWorker::fileType(const DFileInfoPointer &info)
{
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };
    const QUrl &fileUrl = info->uri();
    if (FileUtils::isTrashFile(fileUrl) && info->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()) {
        fileType = FileInfo::FileType::kRegularFile;
        return fileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QString &absoluteFilePath = info->attribute(DFileInfo::AttributeID::kStandardFilePath).toString();
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            fileType = FileInfo::FileType::kDirectory;
        else if (S_ISCHR(statBuffer.st_mode))
            fileType = FileInfo::FileType::kCharDevice;
        else if (S_ISBLK(statBuffer.st_mode))
            fileType = FileInfo::FileType::kBlockDevice;
        else if (S_ISFIFO(statBuffer.st_mode))
            fileType = FileInfo::FileType::kFIFOFile;
        else if (S_ISSOCK(statBuffer.st_mode))
            fileType = FileInfo::FileType::kSocketFile;
        else if (S_ISREG(statBuffer.st_mode))
            fileType = FileInfo::FileType::kRegularFile;
    }
    return fileType;
}

/*!
 * \brief AbstractWorker::startCountProccess start update proccess timer
 */
void AbstractWorker::startCountProccess()
{
    if (!updateProgressTimer)
        updateProgressTimer.reset(new UpdateProgressTimer());
    if (!updateProgressThread)
        updateProgressThread.reset(new QThread);
    updateProgressTimer->moveToThread(updateProgressThread.data());
    updateProgressThread->start();
    connect(this, &AbstractWorker::startUpdateProgressTimer, updateProgressTimer.data(), &UpdateProgressTimer::doStartTime);
    connect(updateProgressTimer.data(), &UpdateProgressTimer::updateProgressNotify, this, &AbstractWorker::onUpdateProgress, Qt::DirectConnection);
    emit startUpdateProgressTimer();
}
/*!
 * \brief AbstractWorker::statisticsFilesSize statistics source files size
 * \return
 */
bool AbstractWorker::statisticsFilesSize()
{
    if (sourceUrls.isEmpty()) {
        fmWarning() << "sources files list is empty!";
        return false;
    }

    const QUrl &firstUrl = sourceUrls.first();

    if (this->targetUrl.isValid()) {
        supportDfmioCopy = DeviceUtils::supportDfmioCopyDevice(this->targetUrl)
                || DeviceUtils::supportDfmioCopyDevice(firstUrl);
        supportSetPermission = DeviceUtils::supportSetPermissionsDevice(this->targetUrl);
    }
    // 判读源文件所在设备位置，执行异步或者同统计源文件大小
    isSourceFileLocal = FileOperationsUtils::isFileOnDisk(firstUrl);

    if (isSourceFileLocal) {
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(firstUrl);
        isSourceFileLocal = fsType.startsWith("ext");
    }

    if (isSourceFileLocal) {
        const SizeInfoPointer &fileSizeInfo = FileOperationsUtils::statisticsFilesSize(sourceUrls, true);
        allFilesList = fileSizeInfo->allFiles;
        sourceFilesTotalSize = fileSizeInfo->totalSize;
        workData->dirSize = fileSizeInfo->dirSize;
        sourceFilesCount = fileSizeInfo->fileCount;
    } else {
        statisticsFilesSizeJob.reset(new DFMBASE_NAMESPACE::FileStatisticsJob());
        connect(statisticsFilesSizeJob.data(), &DFMBASE_NAMESPACE::FileStatisticsJob::finished,
                this, &AbstractWorker::onStatisticsFilesSizeFinish, Qt::DirectConnection);
        connect(statisticsFilesSizeJob.data(), &DFMBASE_NAMESPACE::FileStatisticsJob::sizeChanged, this, &AbstractWorker::onStatisticsFilesSizeUpdate, Qt::DirectConnection);
        statisticsFilesSizeJob->start(sourceUrls);
    }
    return true;
}
/*!
 * \brief AbstractWorker::copyWait Blocking waiting for task
 * \return Is it running
 */
bool AbstractWorker::workerWait()
{
    waitCondition.wait(&mutex);

    return currentState == AbstractJobHandler::JobState::kRunningState;
}
/*!
 * \brief AbstractWorker::setStat Set current task status
 * \param stat task status
 */
void AbstractWorker::setStat(const AbstractJobHandler::JobState &stat)
{
    if (stat == AbstractJobHandler::JobState::kRunningState)
        waitCondition.wakeAll();

    if (stat == currentState)
        return;

    currentState = stat;

    emitStateChangedNotify();
}
/*!
 * \brief AbstractWorker::initArgs init job agruments
 * \return
 */
bool AbstractWorker::initArgs()
{
    sourceFilesTotalSize = 0;
    setStat(AbstractJobHandler::JobState::kRunningState);
    if (!localFileHandler)
        localFileHandler.reset(new LocalFileHandler);
    completeSourceFiles.clear();
    completeTargetFiles.clear();
    completeCustomInfos.clear();
    bigFileSize = FileOperationsUtils::bigFileSize();

    return true;
}
/*!
 * \brief AbstractWorker::endWork end task and emit task finished
 */
void AbstractWorker::endWork()
{
    setStat(AbstractJobHandler::JobState::kStopState);

    Q_EMIT removeTaskWidget();

    // send finish signal
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(completeSourceFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey, QVariant::fromValue(completeTargetFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey, QVariant::fromValue(completeCustomInfos));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));

    saveOperations();

    emit finishedNotify(info);

    fmInfo() << "\n work end, job: " << jobType
             << "\n sources parent: " << (sourceUrls.count() <= 0 ? QUrl() : parentUrl(sourceUrls.first()))
             << "\n sources count: " << sourceUrls.count()
             << "\n target: " << targetUrl
             << "\n time elapsed: " << timeElapsed.elapsed()
             << "\n";
    fmDebug() << "\n sources urls: " << sourceUrls;
    if (statisticsFilesSizeJob) {
        statisticsFilesSizeJob->stop();
        statisticsFilesSizeJob->wait();
    }

    emit workerFinish();
}
/*!
 * \brief AbstractWorker::emitStateChangedNotify send state changed signal
 */
void AbstractWorker::emitStateChangedNotify()
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));

    emit stateChangedNotify(info);
}
/*!
 * \brief AbstractWorker::emitCurrentTaskNotify send current task information
 * \param from source url
 * \param to target url
 */
void AbstractWorker::emitCurrentTaskNotify(const QUrl &from, const QUrl &to)
{
    auto fromUrl = from;
    fromUrl.setPath(QUrl::fromPercentEncoding(QByteArray(from.path().toStdString().data())));
    JobInfoPointer info = createCopyJobInfo(fromUrl, to);

    emit currentTaskNotify(info);
}
/*!
 * \brief AbstractWorker::emitProgressChangedNotify send process changed signal
 * \param writSize task complete data size
 */
void AbstractWorker::emitProgressChangedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    if (AbstractJobHandler::JobType::kCopyType == jobType
        || AbstractJobHandler::JobType::kCutType == jobType) {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceFilesTotalSize)));
    } else if (AbstractJobHandler::JobType::kMoveToTrashType == jobType
               || AbstractJobHandler::JobType::kRestoreType == jobType) {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceUrls.count())));
    } else {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(allFilesList.count())));
    }
    AbstractJobHandler::StatisticState state = AbstractJobHandler::StatisticState::kNoState;
    if (statisticsFilesSizeJob) {
        if (statisticsFilesSizeJob->isFinished())
            state = AbstractJobHandler::StatisticState::kStopState;
        else
            state = AbstractJobHandler::StatisticState::kRunningState;
    }
    info->insert(AbstractJobHandler::NotifyInfoKey::kStatisticStateKey, QVariant::fromValue(state));

    info->insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey, QVariant::fromValue(writSize));

    emit progressChangedNotify(info);
}
/*!
 * \brief AbstractWorker::emitErrorNotify send job error signal
 * \param from source url
 * \param to target url
 * \param error task error type
 * \param errorMsg task error message
 */
void AbstractWorker::emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const bool isTo,
                                     const quint64 id, const QString &errorMsg, const bool allUsErrorMsg)
{
    auto fromUrl = from;
    fromUrl.setPath(QUrl::fromPercentEncoding(QByteArray(from.path().toStdString().data())));
    JobInfoPointer info = createCopyJobInfo(fromUrl, to, error);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(error));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey,
                 QVariant::fromValue(ErrorMessageAndAction::errorMsg(fromUrl, to, error, isTo, errorMsg, allUsErrorMsg)));
    info->insert(AbstractJobHandler::NotifyInfoKey::kActionsKey,
                 QVariant::fromValue(ErrorMessageAndAction::supportActions(error)));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey, QVariant::fromValue(fromUrl));
    quint64 emitId = id == 0 ? quintptr(this) : id;
    info->insert(AbstractJobHandler::NotifyInfoKey::kWorkerPointer, QVariant::fromValue(emitId));
    emit errorNotify(info);

    fmDebug() << "work error, job: " << jobType << " job error: " << error << " url from: " << from << " url to: " << to
              << " error msg: " << errorMsg << id;
}

/*!
 * \brief AbstractWorker::isStopped current task is stopped
 * \return current task is stopped
 */
bool AbstractWorker::isStopped()
{
    return AbstractJobHandler::JobState::kStopState == currentState;
}
/*!
 * \brief AbstractWorker::createCopyJobInfo create signal agruments information
 * \param from source url
 * \param to from url
 * \return signal agruments information
 */
JobInfoPointer AbstractWorker::createCopyJobInfo(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType error)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey, QVariant::fromValue(from));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetUrlKey, QVariant::fromValue(to));
    QString fromMsg, toMsg;
    ErrorMessageAndAction::srcAndDestString(from, to, &fromMsg, &toMsg, jobType, error);
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, QVariant::fromValue(fromMsg));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, QVariant::fromValue(toMsg));
    return info;
}

void AbstractWorker::resumeAllThread()
{
    resume();
    if (copyOtherFileWorker)
        copyOtherFileWorker->resume();
    for (auto worker : threadCopyWorker) {
        worker->resume();
    }
}

void AbstractWorker::resumeThread(const QList<quint64> &errorIds)
{
    if (!errorIds.contains(quintptr(this)) && (!copyOtherFileWorker || !errorIds.contains(quintptr(copyOtherFileWorker.data()))))
        resume();

    for (auto worker : threadCopyWorker) {
        if (!errorIds.contains(quintptr(worker.data())))
            worker->resume();
    }
}

void AbstractWorker::pauseAllThread()
{
    pause();
    if (copyOtherFileWorker)
        copyOtherFileWorker->pause();
    for (auto worker : threadCopyWorker) {
        worker->pause();
    }
}

void AbstractWorker::stopAllThread()
{
    if (copyOtherFileWorker)
        copyOtherFileWorker->stop();
    for (auto worker : threadCopyWorker) {
        worker->stop();
    }
    stop();
}

void AbstractWorker::checkRetry()
{
    if (workData->signalThread || !retry)
        return;
    emit retryErrSuccess(quintptr(this));
}
/*!
 * \brief AbstractWorker::doWork task Thread execution
 * \return
 */
bool AbstractWorker::doWork()
{
    timeElapsed.start();
    fmDebug() << "\n=========================\nwork begin, job: " << jobType << " sources: " << sourceUrls << " target: " << targetUrl << "\n";

    // 执行拷贝的业务逻辑
    if (!initArgs()) {
        endWork();
        return false;
    }
    // 统计文件总大小
    if (!statisticsFilesSize()) {
        endWork();
        return false;
    }
    // 启动统计写入数据大小计时器
    startCountProccess();

    return true;
}
/*!
 * \brief AbstractWorker::stateCheck Blocking waiting for task and check status
 * \return is Correct state
 */
bool AbstractWorker::stateCheck()
{
    if (currentState == AbstractJobHandler::JobState::kRunningState) {
        return true;
    }
    if (currentState == AbstractJobHandler::JobState::kPauseState) {
        fmInfo() << "Will be suspended";
        if (!workerWait()) {
            return currentState != AbstractJobHandler::JobState::kStopState;
        }
    } else if (currentState == AbstractJobHandler::JobState::kStopState) {
        return false;
    }

    return true;
}
/*!
 * \brief AbstractWorker::onStatisticsFilesSizeFinish  Count the size of all files
 * and the slot at the end of the thread
 * \param sizeInfo All file size information
 */
void AbstractWorker::onStatisticsFilesSizeFinish()
{
    statisticsFilesSizeJob->stop();
    const SizeInfoPointer &sizeInfo = statisticsFilesSizeJob->getFileSizeInfo();
    sourceFilesTotalSize = statisticsFilesSizeJob->totalProgressSize();
    workData->dirSize = sizeInfo->dirSize;
    sourceFilesCount = sizeInfo->fileCount;
    allFilesList = sizeInfo->allFiles;
}

void AbstractWorker::onStatisticsFilesSizeUpdate(qint64 size)
{
    sourceFilesTotalSize = size;
}

AbstractWorker::AbstractWorker(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType>();
}
/*!
 * \brief AbstractWorker::formatFileName Processing and formatting file names
 * \param fileName file name
 * \return format file name
 */
QString AbstractWorker::formatFileName(const QString &fileName)
{
    // 获取目标文件的文件系统，是vfat格式是否要特殊处理，以前的文管处理的
    if (workData->jobFlags.testFlag(AbstractJobHandler::JobFlag::kDontFormatFileName)) {
        return fileName;
    }

    const QString &fs_type = QStorageInfo(targetUrl.path()).fileSystemType();

    if (fs_type == "vfat") {
        QString new_name = fileName;

        return new_name.replace(QRegularExpression("[\"*:<>?\\|]"), "_");
    }

    return fileName;
}

void AbstractWorker::saveOperations()
{
    if (!isConvert && !completeTargetFiles.isEmpty()) {
        // send saveoperator event
        if (jobType == AbstractJobHandler::JobType::kCopyType
            || jobType == AbstractJobHandler::JobType::kCutType
            || jobType == AbstractJobHandler::JobType::kMoveToTrashType
            || jobType == AbstractJobHandler::JobType::kRestoreType) {
            GlobalEventType operatorType = GlobalEventType::kDeleteFiles, redoType = GlobalEventType::kUnknowType;
            QList<QUrl> targetUrls, redoSources, redoTargets;
            redoSources = completeSourceFiles;
            redoTargets.append(targetUrl);
            switch (jobType) {
            case AbstractJobHandler::JobType::kCopyType:
                operatorType = GlobalEventType::kDeleteFiles;
                targetUrls.append(parentUrl(completeSourceFiles.first()));
                redoType = GlobalEventType::kCopy;
                break;
            case AbstractJobHandler::JobType::kCutType:
                operatorType = GlobalEventType::kCutFile;
                if (!sourceUrls.isEmpty() && FileUtils::isTrashFile(sourceUrls.first())) {
                    operatorType = GlobalEventType::kMoveToTrash;
                } else {
                    targetUrls.append(parentUrl(completeSourceFiles.first()));
                }
                redoType = GlobalEventType::kCutFile;
                break;
            case AbstractJobHandler::JobType::kMoveToTrashType:
                operatorType = GlobalEventType::kRestoreFromTrash;
                redoType = GlobalEventType::kMoveToTrash;
                break;
            case AbstractJobHandler::JobType::kRestoreType:
                operatorType = GlobalEventType::kMoveToTrash;
                redoType = GlobalEventType::kRestoreFromTrash;
                break;
            default:
                operatorType = GlobalEventType::kUnknowType;
                break;
            }
            QVariantMap values;
            values.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(operatorType)));
            values.insert("undosources", QUrl::toStringList(completeTargetFiles));
            values.insert("undotargets", QUrl::toStringList(targetUrls));
            values.insert("redoevent", QVariant::fromValue(static_cast<uint16_t>(redoType)));
            values.insert("redosources", QUrl::toStringList(completeSourceFiles));
            values.insert("redotargets", QUrl::toStringList(redoTargets));
            dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, values);
        }
    }

    if (handle && isConvert && !completeSourceFiles.isEmpty()) {
        emit requestSaveRedoOperation(QString::number(quintptr(handle.data()), 16), deleteFirstFileSize.load());
    }
    if (jobType == AbstractJobHandler::JobType::kCopyType
            || jobType == AbstractJobHandler::JobType::kCutType
            || FileOperationsUtils::canBroadcastPaste()) {
        QUrl sourceUrl = sourceUrls.isEmpty() ? QUrl() : sourceUrls.first();
        if (!sourceUrl.isValid() || !targetUrl.isValid()) {
            fmWarning(logDFMBase()) << "broadcast paste error, cast invalid source or target url!!!"
                                    << sourceUrl << targetUrl;
            return;
        }
        sourceUrl = parentUrl(sourceUrl);
        UniversalUtils::boardCastPastData(sourceUrl, targetUrl, completeSourceFiles);
    }
}

AbstractWorker::~AbstractWorker()
{
    if (statisticsFilesSizeJob) {
        statisticsFilesSizeJob->stop();
        statisticsFilesSizeJob->wait();
    }
    if (speedtimer) {
        delete speedtimer;
        speedtimer = nullptr;
    }

    qDeleteAll(speedtimerList);
}

/*!
 * \brief AbstractWorker::initHandleConnects 初始化当前信号的连接
 * \param handle 任务控制处理器
 */
void AbstractWorker::initHandleConnects(const JobHandlePointer handle)
{
    if (!handle) {
        fmWarning() << "JobHandlePointer is a nullptr,so connects failed!";
        return;
    }
    connect(this, &AbstractWorker::progressChangedNotify, handle.get(), &AbstractJobHandler::onProccessChanged, Qt::QueuedConnection);
    connect(this, &AbstractWorker::stateChangedNotify, handle.get(), &AbstractJobHandler::onStateChanged, Qt::QueuedConnection);
    connect(this, &AbstractWorker::finishedNotify, handle.get(), &AbstractJobHandler::onFinished, Qt::QueuedConnection);
    connect(this, &AbstractWorker::removeTaskWidget, handle.get(), &AbstractJobHandler::requestRemoveTaskWidget, Qt::QueuedConnection);
    connect(this, &AbstractWorker::speedUpdatedNotify, handle.get(), &AbstractJobHandler::onSpeedUpdated, Qt::QueuedConnection);
    connect(this, &AbstractWorker::currentTaskNotify, handle.get(), &AbstractJobHandler::onCurrentTask, Qt::QueuedConnection);
    connect(this, &AbstractWorker::requestTaskDailog, handle.get(), &AbstractJobHandler::requestTaskDailog, Qt::QueuedConnection);
}
