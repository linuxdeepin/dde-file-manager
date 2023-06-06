// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burnjob.h"
#include "utils/burnhelper.h"
#include "utils/burnsignalmanager.h"
#include "utils/burncheckstrategy.h"
#include "events/burneventcaller.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QDebug>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE
DFM_BURN_USE_NS
using namespace GlobalServerDefines;

static constexpr int kPipeBufferSize { 4096 };
static constexpr char kMapKeyStatus[] { "status" };
static constexpr char kMapKeyProgress[] { "progress" };
static constexpr char kMapKeySpeed[] { "speed" };
static constexpr char kMapKeyMsg[] { "msg" };
static constexpr char kMapKeyPhase[] { "phase" };

AbstractBurnJob::AbstractBurnJob(const QString &dev, const JobHandlePointer handler)
    : curDev(dev), jobHandlePtr(handler)
{
    connect(DevMngIns, &DeviceManager::blockDevPropertyChanged,
            this, [](const QString &deviceId, const QString &property, const QVariant &val) {
                // TODO(zhangs): mediaChangeDetected
            });

    connect(BurnSignalManager::instance(), &BurnSignalManager::activeTaskDialog, this, &AbstractBurnJob::addTask);
}

QVariantMap AbstractBurnJob::currentDeviceInfo() const
{
    return curDeviceInfo;
}

QVariant AbstractBurnJob::property(AbstractBurnJob::PropertyType type) const
{
    return curProperty[type];
}

void AbstractBurnJob::setProperty(AbstractBurnJob::PropertyType type, const QVariant &val)
{
    curProperty[type] = val;
}

void AbstractBurnJob::addTask()
{
    if (jobHandlePtr) {
        DialogManagerInstance->addTask(jobHandlePtr);
        JobInfoPointer info { new QMap<quint8, QVariant> };
        info->insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey, lastProgress);
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, 100);
        emit jobHandlePtr->proccessChangedNotify(info);
    }
}

bool AbstractBurnJob::fileSystemLimitsValid()
{
    return true;
}

void AbstractBurnJob::updateMessage(JobInfoPointer ptr)
{
    Q_ASSERT(ptr);
    if (curJobType != JobType::kOpticalBlank) {
        const QString &msgSource { tr("Burning disc %1, please wait...").arg(curDev) };
        QString msgTarget { tr("Writing data...") };
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, msgSource);
        if (curJobType == JobType::kOpticalCheck)
            msgTarget = tr("Verifying data...");
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, msgTarget);
        emit jobHandlePtr->currentTaskNotify(ptr);
    }
}

void AbstractBurnJob::updateSpeed(JobInfoPointer ptr, JobStatus status, const QString &speed)
{
    if (status == JobStatus::kRunning)
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, speed);
    else
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, "");
    emit jobHandlePtr->speedUpdatedNotify(ptr);
}

void AbstractBurnJob::readFunc(int progressFd, int checkFd)
{
    while (true) {
        char buf[kPipeBufferSize] { 0 };
        if (read(progressFd, buf, kPipeBufferSize) <= 0) {
            qDebug() << "progressFd break";
            break;
        } else {
            QByteArray bufByes(buf);
            qDebug() << "burn files, read bytes json:" << bufByes;
            QJsonParseError jsonError;
            QJsonObject obj { QJsonDocument::fromJson(bufByes, &jsonError).object() };
            if (jsonError.error == QJsonParseError::NoError) {
                int stat { obj[kMapKeyStatus].toInt() };
                int progress { obj[kMapKeyProgress].toInt() };
                QString &&speed = obj[kMapKeySpeed].toString();
                QJsonArray &&jsonArray = obj[kMapKeyMsg].toArray();
                int phase { obj[kMapKeyPhase].toInt() };
                if (phase != curPhase) {
                    curPhase = phase;
                    lastProgress = 0;
                    if (curPhase == JobPhase::kCheckData)
                        curJobType = JobType::kOpticalCheck;
                }
                QStringList msgList;
                for (int i = 0; i < jsonArray.size(); i++)
                    msgList.append(jsonArray[i].toString());

                onJobUpdated(static_cast<JobStatus>(stat), progress, speed, msgList);
            }
        }
    }

    if (lastStatus != JobStatus::kIdle)
        comfort();

    // check
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };
    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    double bad {};
    if (check && lastStatus != JobStatus::kFailed)
        read(checkFd, &bad, sizeof(bad));
    bool checkRet { !(check && (bad > (2 + 1e-6))) };

    // show result dialog
    finishFunc(check, checkRet);
}

void AbstractBurnJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(progressFd)
    Q_UNUSED(checkFd)
}

void AbstractBurnJob::finishFunc(bool verify, bool verifyRet)
{
    if (lastStatus == JobStatus::kFailed) {
        jobSuccess = false;
        if (verify && verifyRet)
            emit requestCompletionDialog(tr("Data verification successful."), "dialog-ok");
        else
            emit requestFailureDialog(static_cast<int>(curJobType), lastError, lastSrcMessages);
    } else {
        jobSuccess = true;
        if (verify)
            emit requestCompletionDialog(tr("Data verification successful."), "dialog-ok");
        else
            emit requestCompletionDialog(tr("Burn process completed"), "dialog-ok");
    }

    emit burnFinished(firstJobType, jobSuccess);
    DeviceManager::instance()->ejectBlockDevAsync(curDevId);
}

void AbstractBurnJob::run()
{
    curDevId = { DeviceUtils::getBlockDeviceId(curDev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };
    BurnHelper::updateBurningStateToPersistence(curDevId, curDev, true);
    FinallyUtil finaly([this]() {
        BurnHelper::updateBurningStateToPersistence(curDevId, curDev, false);
    });

    work();

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(jobHandlePtr));
    emit jobHandlePtr->requestRemoveTaskWidget();
    emit jobHandlePtr->finishedNotify(info);
}

bool AbstractBurnJob::readyToWork()
{
    curDeviceInfo = DevProxyMng->queryBlockInfo(curDevId);
    auto &&map { curDeviceInfo };
    if (map.isEmpty()) {
        qWarning() << "Device info is empty";
        return false;
    }

    bool blank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    // for dvd+rw/dvd-rw disc, erase operation only overwrite some blocks which used to present filesystem,
    // so the blank field is still false even if it can be write datas from the beginning,
    auto mediaType { static_cast<MediaType>(map[DeviceProperty::kOpticalMediaType].toUInt()) };
    if (mediaType == MediaType::kDVD_PLUS_RW || mediaType == MediaType::kDVD_RW)
        blank |= map[DeviceProperty::kSizeTotal].toULongLong() == map[DeviceProperty::kSizeFree].toULongLong();

    if (blank) {
        QString tag = curDevId.mid(curDevId.lastIndexOf("/") + 1);
        QUrl url(QString("burn:///dev/%1/disc_files/").arg(tag));
        emit requestCloseTab(url);
    } else {
        QString mpt { qvariant_cast<QString>(map[DeviceProperty::kMountPoint]) };
        if (!mpt.isEmpty()) {
            if (!DeviceManager::instance()->unmountBlockDev(curDevId)) {
                emit requestErrorMessageDialog(tr("The device was not safely unmounted"), tr("Disk is busy, cannot unmount now"));
                return false;
            }
        }
    }

    return true;
}

void AbstractBurnJob::workingInSubProcess()
{
    int progressPipefd[2] {};
    if (pipe(progressPipefd) < 0) {
        qWarning() << "pipe failed";
        return;
    }

    int badPipefd[2] {};
    if (pipe(badPipefd) < 0) {
        qWarning() << "pipe failed";
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {   // child process: working
        close(progressPipefd[0]);
        close(badPipefd[0]);

        writeFunc(progressPipefd[1], badPipefd[1]);

        close(progressPipefd[1]);
        close(badPipefd[1]);
        _exit(0);
    } else if (pid > 0) {   // parent process: wait and notify
        close(progressPipefd[1]);
        close(badPipefd[1]);

        int status;
        waitpid(-1, &status, WNOHANG);
        qDebug() << "start read child process data";
        QThread::msleep(1000);

        readFunc(progressPipefd[0], badPipefd[0]);

        close(progressPipefd[0]);
        close(badPipefd[0]);
    } else {
        qWarning() << "fork failed";
    }
}

DOpticalDiscManager *AbstractBurnJob::createManager(int fd)
{
    DOpticalDiscManager *manager = new DOpticalDiscManager(curDev, this);
    connect(
            manager, &DOpticalDiscManager::jobStatusChanged, this,
            [=](DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message) {
                QByteArray bytes(updatedInSubProcess(status, progress, speed, message));
                if (bytes.size() < kPipeBufferSize) {
                    char progressBuf[kPipeBufferSize] = { 0 };
                    strncpy(progressBuf, bytes.data(), kPipeBufferSize);
                    write(fd, progressBuf, strlen(progressBuf) + 1);
                }
            },
            Qt::DirectConnection);
    return manager;
}

QByteArray AbstractBurnJob::updatedInSubProcess(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    QJsonObject obj;
    obj[kMapKeyStatus] = int(status);
    obj[kMapKeyProgress] = progress;
    obj[kMapKeySpeed] = speed;
    obj[kMapKeyMsg] = QJsonArray::fromStringList(message);
    obj[kMapKeyPhase] = curPhase;
    return QJsonDocument(obj).toJson();
}

void AbstractBurnJob::comfort()
{
    // must show %100, this psychological comfort
    auto tmp = lastStatus;
    if (lastStatus != JobStatus::kFailed) {
        for (int i = 0; i != 10; i++) {
            onJobUpdated(JobStatus::kRunning, 100, "", {});
            QThread::msleep(100);
        }
    }
    lastStatus = tmp;
}

void AbstractBurnJob::onJobUpdated(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    lastStatus = status;

    Q_ASSERT(jobHandlePtr);
    JobInfoPointer info { new QMap<quint8, QVariant> };

    // hide btn
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateHideKey, true);
    emit jobHandlePtr->stateChangedNotify(info);

    // group `Persistence::kBurnStateGroup` will be deleted when filemanger starts
    if (!Application::dataPersistence()->groups().contains(Persistence::kBurnStateGroup))
        BurnHelper::updateBurningStateToPersistence(curDevId, curDev, true);

    // update progress
    if (progress > 0 && progress <= 100 && progress >= lastProgress) {
        lastProgress = progress;
        info->insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey, progress);
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, 100);
        emit jobHandlePtr->proccessChangedNotify(info);
    }

    // show failed dialog
    if (status == JobStatus::kFailed) {
        lastSrcMessages = message;
        lastError = BurnHelper::parseXorrisoErrorMessage(message);
        return;
    }

    // update message
    updateMessage(info);
    // update speed
    updateSpeed(info, status, speed);
}

EraseJob::EraseJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

void EraseJob::updateMessage(JobInfoPointer ptr)
{
    Q_ASSERT(ptr);
    if (curJobType == JobType::kOpticalBlank) {
        const QString &msg { tr("Erasing disc %1, please wait...").arg(curDev) };
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, msg);
        emit jobHandlePtr->currentTaskNotify(ptr);
    }
}

void EraseJob::work()
{
    qInfo() << "Start erase device: " << curDev;

    // TODO(zhangs): check unmount
    firstJobType = curJobType = JobType::kOpticalBlank;
    if (!readyToWork())
        return;

    DOpticalDiscManager *manager = new DOpticalDiscManager(curDev, this);
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    connect(manager, &DOpticalDiscManager::jobStatusChanged, this, &AbstractBurnJob::onJobUpdated, Qt::DirectConnection);

    if (!manager->erase())
        qWarning() << "Erase Failed: " << manager->lastError();
    qInfo() << "End erase device: " << curDev;

    comfort();
    DeviceManager::instance()->rescanBlockDev(curDevId);

    // Due to disc don't ejected after erase, we must readlod optical info again
    DevMngIns->mountBlockDevAsync(curDevId, {}, [this](bool, const DFMMOUNT::OperationErrorInfo &, const QString &) {
        DevProxyMng->reloadOpticalInfo(curDevId);
    });
}

BurnISOFilesJob::BurnISOFilesJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

bool BurnISOFilesJob::fileSystemLimitsValid()
{
    auto stagingurl { curProperty[PropertyType::KStagingUrl].toUrl() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    // filesystem limits check
    BurnCheckStrategy *checkStrategy { nullptr };
    if (opts.testFlag(DFMBURN::BurnOption::kISO9660Only))
        checkStrategy = new ISO9660CheckStrategy(stagingurl.path(), this);
    else if (opts.testFlag(DFMBURN::BurnOption::kJolietSupport))
        checkStrategy = new JolietCheckStrategy(stagingurl.path(), this);
    else
        checkStrategy = new RockRidgeCheckStrategy(stagingurl.path(), this);

    if (checkStrategy && !checkStrategy->check()) {
        qWarning() << "Check Failed: " << checkStrategy->lastError();
        emit requestErrorMessageDialog(tr("The file name or the path is too long. Please shorten the file name or the path and try again."),
                                       checkStrategy->lastInvalidName());
        return false;
    }

    return true;
}

void BurnISOFilesJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::KStagingUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto volName { curProperty[PropertyType::kVolumeName].toString() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString localPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    manager->setStageFile(localPath);
    curPhase = kWriteData;
    bool isSuccess { manager->commit(opts, speeds, volName) };
    qInfo() << "Burn ret: " << isSuccess << manager->lastError() << localPath;
    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
}

void BurnISOFilesJob::work()
{
    qInfo() << "Start burn ISO files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;
    if (!fileSystemLimitsValid())
        return;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End burn ISO files: " << curDev;
}

BurnISOImageJob::BurnISOImageJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

void BurnISOImageJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::kImageUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString imgPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    curPhase = kWriteData;
    bool isSuccess { manager->writeISO(imgPath, speeds) };
    qInfo() << "Burn ISO ret: " << isSuccess << manager->lastError() << imgPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
}

void BurnISOImageJob::work()
{
    qInfo() << "Start burn ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End burn ISO image: " << curDev;
}

BurnUDFFilesJob::BurnUDFFilesJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

bool BurnUDFFilesJob::fileSystemLimitsValid()
{
    auto stagingurl { curProperty[PropertyType::KStagingUrl].toUrl() };

    // filesystem limits check
    BurnCheckStrategy *checkStrategy { new UDFCheckStrategy(stagingurl.path(), this) };
    if (!checkStrategy->check()) {
        qWarning() << "Check Failed: " << checkStrategy->lastError();
        emit requestErrorMessageDialog(tr("The file name or the path is too long. Please shorten the file name or the path and try again."),
                                       checkStrategy->lastInvalidName());
        return false;
    }

    return true;
}

void BurnUDFFilesJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(checkFd);
    auto url { curProperty[PropertyType::KStagingUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto volName { curProperty[PropertyType::kVolumeName].toString() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString localPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    manager->setStageFile(localPath);
    curPhase = kWriteData;
    bool isSuccess { manager->commit(opts, speeds, volName) };
    qInfo() << "Burn UDF ret: " << isSuccess << manager->lastError() << localPath;
}

void BurnUDFFilesJob::work()
{
    qInfo() << "Start burn UDF files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;
    if (!fileSystemLimitsValid())
        return;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End burn UDF files: " << curDev;
}

DumpISOImageJob::DumpISOImageJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)

{
}

void DumpISOImageJob::updateMessage(JobInfoPointer ptr)
{
    Q_ASSERT(ptr);
    if (curJobType == JobType::kOpticalImageDump) {
        const auto &url { curProperty[PropertyType::kImageUrl].toUrl() };
        const QString &imagePath { url.toLocalFile() };
        const QString &msgSource { tr("Creating an ISO image") };
        const QString &msgTarget { tr("to %1").arg(imagePath) };
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, msgSource);
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, msgTarget);
        emit jobHandlePtr->currentTaskNotify(ptr);
    }
}

void DumpISOImageJob::updateSpeed(JobInfoPointer ptr, JobStatus status, const QString &speed)
{
    Q_UNUSED(status)
    Q_UNUSED(speed)
    ptr->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, "");
    emit jobHandlePtr->speedUpdatedNotify(ptr);
}

void DumpISOImageJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(checkFd)
    const auto &url { curProperty[PropertyType::kImageUrl].toUrl() };
    const QString &imagePath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    curPhase = kWriteData;
    bool isSuccess { manager->dumpISO(imagePath) };
    qInfo() << "Dump ISO ret: " << isSuccess << manager->lastError() << imagePath;
}

void DumpISOImageJob::finishFunc(bool verify, bool verifyRet)
{
    Q_UNUSED(verify)
    Q_UNUSED(verifyRet)

    if (lastStatus == JobStatus::kFailed || lastStatus == JobStatus::kIdle) {
        jobSuccess = false;
        emit requestOpticalDumpISOFailedDialog();
    } else {
        jobSuccess = true;
        const auto &url { curProperty[PropertyType::kImageUrl].toUrl() };
        emit requestOpticalDumpISOSuccessDialog(url);
    }
}

void DumpISOImageJob::work()
{
    qInfo() << "Start dump ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageDump;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End dump ISO image: " << curDev;
}
