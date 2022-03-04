/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "abstractworker.h"

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QWaitCondition>
#include <QMutex>
#include <QApplication>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QDebug>

DSC_USE_NAMESPACE
/*!
 * \brief setWorkArgs 设置当前任务的参数
 * \param args 参数
 */
void AbstractWorker::setWorkArgs(const JobHandlePointer &handle, const QList<QUrl> &sources, const QUrl &target,
                                 const AbstractJobHandler::JobFlags &flags)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr, setWorkArgs failed!";
        return;
    }
    this->handle = handle;
    initHandleConnects(handle);
    this->sourceUrls = sources;
    this->targetUrl = target;
    isConvert = flags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kRevocation);
    jobFlags = flags;
}

/*!
 * \brief doOperateWork 处理用户的操作 不在拷贝线程执行的函数，协同类直接调用
 * \param actions 当前操作
 */
void AbstractWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)) {
        stop();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)) {
        pause();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kResumAction)) {
        resume();
    } else {
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
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kEnforceAction)) {
            currentAction = AbstractJobHandler::SupportAction::kEnforceAction;
        } else {
            currentAction = AbstractJobHandler::SupportAction::kNoAction;
        }

        handlingErrorCondition.wakeAll();
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
    // clean error info queue
    {
        QMutexLocker lk(&errorThreadIdQueueMutex);
        errorThreadIdQueue.clear();
    }

    waitCondition.wakeAll();

    handlingErrorCondition.wakeAll();

    errorCondition.wakeAll();

    if (updateProccessTimer)
        updateProccessTimer->stopTimer();

    if (updateProccessThread) {
        updateProccessThread->quit();
        updateProccessThread->wait();
    }
}
/*!
 * \brief AbstractWorker::pause paused task
 */
void AbstractWorker::pause()
{
    if (currentState == AbstractJobHandler::JobState::kPauseState)
        return;
    setStat(AbstractJobHandler::JobState::kPauseState);
}
/*!
 * \brief AbstractWorker::resume resume task
 */
void AbstractWorker::resume()
{
    setStat(AbstractJobHandler::JobState::kRunningState);

    waitCondition.wakeAll();

    errorCondition.wakeAll();
}
/*!
 * \brief AbstractWorker::startCountProccess start update proccess timer
 */
void AbstractWorker::startCountProccess()
{
    if (!updateProccessTimer)
        updateProccessTimer.reset(new UpdateProccessTimer());
    if (!updateProccessThread)
        updateProccessThread.reset(new QThread);
    updateProccessTimer->moveToThread(updateProccessThread.data());
    updateProccessThread->start();
    connect(this, &AbstractWorker::startUpdateProccessTimer, updateProccessTimer.data(), &UpdateProccessTimer::doStartTime);
    connect(updateProccessTimer.data(), &UpdateProccessTimer::updateProccessNotify, this, &AbstractWorker::onUpdateProccess, Qt::DirectConnection);
    emit startUpdateProccessTimer();
}
/*!
 * \brief AbstractWorker::statisticsFilesSize statistics source files size
 * \return
 */
bool AbstractWorker::statisticsFilesSize()
{
    if (sourceUrls.isEmpty()) {
        qWarning() << "sources files list is empty!";
        return false;
    }
    // 判读源文件所在设备位置，执行异步或者同统计源文件大小
    isSourceFileLocal = FileOperationsUtils::isFileOnDisk(sourceUrls.at(0));

    if (isSourceFileLocal) {
        QStorageInfo soureStorageInfo(sourceUrls.first().path().toStdString().data());
        isSourceFileLocal = soureStorageInfo.fileSystemType().startsWith("ext");
    }

    if (isSourceFileLocal) {
        const SizeInfoPointer &fileSizeInfo = FileOperationsUtils::statisticsFilesSize(sourceUrls, true);
        allFilesList = fileSizeInfo->allFiles;
        sourceFilesTotalSize = fileSizeInfo->totalSize;
        dirSize = fileSizeInfo->dirSize;
        sourceFilesCount = fileSizeInfo->fileCount;
        return true;
    }

    statisticsFilesSizeJob.reset(new dfmbase::FileStatisticsJob());
    connect(statisticsFilesSizeJob.data(), &dfmbase::FileStatisticsJob::finished, this, &AbstractWorker::onStatisticsFilesSizeFinish);
    statisticsFilesSizeJob->start(sourceUrls);
    return true;
}
/*!
 * \brief AbstractWorker::copyWait Blocking waiting for task
 * \return Is it running
 */
bool AbstractWorker::workerWait()
{
    QMutex lock;
    waitCondition.wait(&lock);
    lock.unlock();

    return currentState == AbstractJobHandler::JobState::kRunningState;
}
/*!
 * \brief AbstractWorker::setStat Set current task status
 * \param stat task status
 */
void AbstractWorker::setStat(const AbstractJobHandler::JobState &stat)
{
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
    sourceFilesTotalSize = -1;
    setStat(AbstractJobHandler::JobState::kRunningState);
    if (!handler)
        handler.reset(new LocalFileHandler);
    completeFiles.clear();
    completeTargetFiles.clear();

    return true;
}
/*!
 * \brief AbstractWorker::endWork end task and emit task finished
 */
void AbstractWorker::endWork()
{
    setStat(AbstractJobHandler::JobState::kStopState);

    // send finish signal
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(completeFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey, QVariant::fromValue(completeTargetFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));

    saveOperations();

    emit finishedNotify(info);
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
    JobInfoPointer info = createCopyJobInfo(from, to);

    emit currentTaskNotify(info);
}
/*!
 * \brief AbstractWorker::emitProccessChangedNotify send process changed signal
 * \param writSize task complete data size
 */
void AbstractWorker::emitProccessChangedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    if (AbstractJobHandler::JobType::kCopyType == jobType) {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceFilesTotalSize)));
    } else {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceFilesCount)));
    }

    info->insert(AbstractJobHandler::NotifyInfoKey::kCurrentProccessKey, QVariant::fromValue(writSize));

    emit proccessChangedNotify(info);
}
/*!
 * \brief AbstractWorker::emitErrorNotify send job error signal
 * \param from source url
 * \param to target url
 * \param error task error type
 * \param errorMsg task error message
 */
void AbstractWorker::emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const QString &errorMsg)
{
    JobInfoPointer info = createCopyJobInfo(from, to);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(error));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey, QVariant::fromValue(errorMsg));
    info->insert(AbstractJobHandler::NotifyInfoKey::kActionsKey, QVariant::fromValue(supportActions(error)));

    emit errorNotify(info);
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
JobInfoPointer AbstractWorker::createCopyJobInfo(const QUrl &from, const QUrl &to)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey, QVariant::fromValue(from));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetUrlKey, QVariant::fromValue(to));
    QString fromMsg, toMsg;
    if (AbstractJobHandler::JobType::kCopyType == jobType) {
        fromMsg = QString(QObject::tr("copy file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kDeleteTpye == jobType) {
        fromMsg = QString(QObject::tr("delete file %1")).arg(from.path());
    } else if (AbstractJobHandler::JobType::kCutType == jobType) {
        fromMsg = QString(QObject::tr("cut file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kMoveToTrashType == jobType) {
        fromMsg = QString(QObject::tr("move file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to trash %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kRestoreType == jobType) {
        fromMsg = QString(QObject::tr("restore file %1 from trash ")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kCleanTrashType == jobType) {
        fromMsg = QString(QObject::tr("clean trash file %1  ")).arg(from.path());
    }
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, QVariant::fromValue(fromMsg));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, QVariant::fromValue(toMsg));
    return info;
}
/*!
 * \brief AbstractWorker::doWork task Thread execution
 * \return
 */
bool AbstractWorker::doWork()
{
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
        qInfo() << "Will be suspended";
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
    sourceFilesTotalSize = sizeInfo->totalSize;
    dirSize = sizeInfo->dirSize;
    sourceFilesCount = sizeInfo->fileCount;
}

AbstractWorker::AbstractWorker(QObject *parent)
    : QObject(parent)
{
}
/*!
 * \brief AbstractWorker::formatFileName Processing and formatting file names
 * \param fileName file name
 * \return format file name
 */
QString AbstractWorker::formatFileName(const QString &fileName)
{
    // 获取目标文件的文件系统，是vfat格式是否要特殊处理，以前的文管处理的
    if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kDontFormatFileName)) {
        return fileName;
    }

    const QString &fs_type = QStorageInfo(targetUrl.path()).fileSystemType();

    if (fs_type == "vfat") {
        QString new_name = fileName;

        return new_name.replace(QRegExp("[\"*:<>?\\|]"), "_");
    }

    return fileName;
}
/*!
 * \brief DoCopyFilesWorker::getNonExistFileName Gets the name of a file that does not exist
 * \param fromInfo Source file information
 * \param targetDir Target directory information
 * \return file name
 */
QString AbstractWorker::getNonExistFileName(const AbstractFileInfoPointer fromInfo, const AbstractFileInfoPointer targetDir)
{
    if (!targetDir || !targetDir->exists()) {
        // TODO:: paused and handle error
        return QString();
    }
    const QString &copy_text = QCoreApplication::translate("DoCopyFilesWorker", "copy",
                                                           "Extra name added to new file name when used for file name.");

    AbstractFileInfoPointer targetFileInfo { nullptr };
    QString fileBaseName = fromInfo->completeBaseName();
    QString suffix = fromInfo->suffix();
    QString fileName = fromInfo->fileName();
    //在7z分卷压缩后的名称特殊处理7z.003
    if (fileName.contains(QRegularExpression(".7z.[0-9]{3,10}$"))) {
        fileBaseName = fileName.left(fileName.indexOf(QRegularExpression(".7z.[0-9]{3,10}$")));
        suffix = fileName.mid(fileName.indexOf(QRegularExpression(".7z.[0-9]{3,10}$")) + 1);
    }

    int number = 0;

    QString newFileName;

    do {
        newFileName = number > 0 ? QString("%1(%2 %3)").arg(fileBaseName, copy_text).arg(number) : QString("%1(%2)").arg(fileBaseName, copy_text);

        if (!suffix.isEmpty()) {
            newFileName.append('.').append(suffix);
        }

        ++number;
        QUrl newUrl;
        newUrl = targetDir->url();
        newUrl.setPath(newUrl.path() + "/" + newFileName);
        targetFileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
    } while (targetFileInfo && targetFileInfo->exists());

    return newFileName;
}

void AbstractWorker::saveOperations()
{
    if (!isConvert && !completeTargetFiles.isEmpty()) {
        // send saveoperator event
        if (jobType == AbstractJobHandler::JobType::kCopyType
            || jobType == AbstractJobHandler::JobType::kCutType
            || jobType == AbstractJobHandler::JobType::kMoveToTrashType
            || jobType == AbstractJobHandler::JobType::kRestoreType) {
            GlobalEventType operatorType = kDeleteFiles;
            QUrl targetUrl;
            switch (jobType) {
            case AbstractJobHandler::JobType::kCopyType:
                operatorType = kDeleteFiles;
                targetUrl = UrlRoute::urlParent(completeFiles.first());
                break;
            case AbstractJobHandler::JobType::kCutType:
                operatorType = kCutFile;
                targetUrl = UrlRoute::urlParent(completeFiles.first());
                break;
            case AbstractJobHandler::JobType::kMoveToTrashType:
                operatorType = kRestoreFromTrash;
                break;
            case AbstractJobHandler::JobType::kRestoreType:
                operatorType = kMoveToTrash;
                break;
            default:
                operatorType = kDeleteFiles;
                break;
            }
            QVariantMap values;
            values.insert("event", QVariant::fromValue(static_cast<uint16_t>(operatorType)));
            QStringList listUrls;
            for (const auto &url : completeTargetFiles) {
                listUrls.append(url.toString());
            }
            values.insert("sources", QVariant::fromValue(listUrls));
            values.insert("target", targetUrl.toString());
            dpfInstance.eventDispatcher().publish(kSaveOperator, values);
        }
    }
}

AbstractWorker::~AbstractWorker()
{
}

/*!
 * \brief AbstractWorker::initHandleConnects 初始化当前信号的连接
 * \param handle 任务控制处理器
 */
void AbstractWorker::initHandleConnects(const JobHandlePointer &handle)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr,so connects failed!";
        return;
    }

    connect(handle.data(), &AbstractJobHandler::userAction, this, &AbstractWorker::doOperateWork, Qt::QueuedConnection);

    connect(this, &AbstractWorker::proccessChangedNotify, handle.data(), &AbstractJobHandler::onProccessChanged,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::stateChangedNotify, handle.data(), &AbstractJobHandler::onStateChanged,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::currentTaskNotify, handle.data(), &AbstractJobHandler::onCurrentTask,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::finishedNotify, handle.data(), &AbstractJobHandler::onFinished,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::errorNotify, handle.data(), &AbstractJobHandler::onError,
            Qt::QueuedConnection);
    connect(this, &AbstractWorker::speedUpdatedNotify, handle.data(), &AbstractJobHandler::onSpeedUpdated,
            Qt::QueuedConnection);
}
