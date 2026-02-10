// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/base/device/private/devicehelper.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-mount/dblockdevice.h>

#include <QDebug>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h> // Required for errno

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
            fmWarning() << "progressFd break";
            break;
        } else {
            QByteArray bufByes(buf);
            fmInfo() << "burn files, read bytes json:" << bufByes;
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
            emit requestCompletionDialog(tr("Data verification successful."), "dde-file-manager");
        else
            emit requestFailureDialog(static_cast<int>(curJobType), lastError, lastSrcMessages);
    } else {
        jobSuccess = true;
        if (verify)
            emit requestCompletionDialog(tr("Data verification successful."), "dde-file-manager");
        else
            emit requestCompletionDialog(tr("Burn process completed"), "dde-file-manager");
    }

    emit burnFinished(firstJobType, jobSuccess);
    DeviceManager::instance()->ejectBlockDevAsync(curDevId);
}

void AbstractBurnJob::run()
{
    curDevId = DeviceUtils::getBlockDeviceId(curDev);
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
        fmWarning() << "Device info is empty";
        return false;
    }

    bool blank { DeviceUtils::isBlankOpticalDisc(curDevId) };
    if (blank) {
        QString tag = curDevId.mid(curDevId.lastIndexOf("/") + 1);
        QUrl url(QString("burn:///dev/%1/disc_files/").arg(tag));
        emit requestCloseTab(url);
    } else {
        QString mpt { qvariant_cast<QString>(map[DeviceProperty::kMountPoint]) };
        if (!mpt.isEmpty()) {
            if (!DeviceManager::instance()->unmountBlockDev(curDevId)) {
                fmWarning() << "The device was not safely unmounted: " << curDevId;
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
        fmWarning() << "pipe failed";
        return;
    }

    int badPipefd[2] {};
    if (pipe(badPipefd) < 0) {
        fmWarning() << "pipe failed";
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

        fmDebug() << "start read child process data, child pid:" << pid;
        QThread::msleep(1000);

        readFunc(progressPipefd[0], badPipefd[0]);

        close(progressPipefd[0]);
        close(badPipefd[0]);

        // Properly wait for the specific child process to prevent zombie process
        int status;
        pid_t result = waitpid(pid, &status, 0);  // Block until child exits
        if (result == pid) {
            if (WIFEXITED(status)) {
                fmDebug() << "Child process exited normally with code:" << WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                fmWarning() << "Child process terminated by signal:" << WTERMSIG(status);
            }
        } else {
            fmWarning() << "waitpid failed for child process:" << pid << "errno:" << errno;
        }
    } else {
        fmCritical() << "fork failed";
    }
}

DOpticalDiscManager *AbstractBurnJob::createManager(int fd)
{
    DOpticalDiscManager *manager = new DOpticalDiscManager(curDev);
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
    obj[kMapKeyMsg] = QJsonArray::fromStringList(message.mid(0, 3));
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

bool AbstractBurnJob::mediaChangDected()
{
    auto blockDev { DeviceHelper::createBlockDevice(curDevId) };
    if (!blockDev)
        return false;

    return blockDev->getProperty(DFMMOUNT::Property::kDriveMediaChangeDetected).toBool();
}

void AbstractBurnJob::onJobUpdated(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    if (!message.isEmpty())
        fmWarning() << "burn job changed, progress:" << progress << "msg:" << message;
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
    fmInfo() << "Start erase device: " << curDev;

    // TODO(zhangs): check unmount
    firstJobType = curJobType = JobType::kOpticalBlank;
    if (!readyToWork())
        return;

    QScopedPointer<DOpticalDiscManager> manager { new DOpticalDiscManager(curDev) };
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    connect(manager.data(), &DOpticalDiscManager::jobStatusChanged, this, &AbstractBurnJob::onJobUpdated, Qt::DirectConnection);

    bool ret { true };
    if (!manager->erase()) {
        ret = false;
        fmWarning() << "Erase Failed: " << manager->lastError();
    }
    fmInfo() << "End erase device: " << curDev;

    if (!mediaChangDected()) {
        ret = false;
        fmWarning() << "Device disconnected:" << curDevId;
        emit requestFailureDialog(static_cast<int>(curJobType), QObject::tr("Device disconnected"), {});
    }

    comfort();
    emit eraseFinished(ret);
    DeviceManager::instance()->rescanBlockDev(curDevId);

    // Due to disc don't ejected after erase, we must readlod optical info again
    emit requestReloadDisc(curDevId);
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
    QScopedPointer<BurnCheckStrategy> checkStrategy { nullptr };
    if (opts.testFlag(DFMBURN::BurnOption::kISO9660Only))
        checkStrategy.reset(new ISO9660CheckStrategy(stagingurl.path()));
    else if (opts.testFlag(DFMBURN::BurnOption::kJolietSupport))
        checkStrategy.reset(new JolietCheckStrategy(stagingurl.path()));
    else
        checkStrategy.reset(new RockRidgeCheckStrategy(stagingurl.path()));

    if (checkStrategy && !checkStrategy->check()) {
        fmWarning() << "Check Failed: " << checkStrategy->lastError();
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
    fmInfo() << "Burn ret: " << isSuccess << manager->lastError() << localPath;
    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        fmInfo() << "Enable check media";
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void BurnISOFilesJob::work()
{
    fmInfo() << "Start burn ISO files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;
    if (!fileSystemLimitsValid())
        return;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    fmInfo() << "End burn ISO files: " << curDev;
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
    fmInfo() << "Burn ISO ret: " << isSuccess << manager->lastError() << imgPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        fmInfo() << "Enable check media";
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void BurnISOImageJob::work()
{
    fmInfo() << "Start burn ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    fmInfo() << "End burn ISO image: " << curDev;
}

BurnUDFFilesJob::BurnUDFFilesJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

bool BurnUDFFilesJob::fileSystemLimitsValid()
{
    auto stagingurl { curProperty[PropertyType::KStagingUrl].toUrl() };

    // filesystem limits check
    QScopedPointer<BurnCheckStrategy> checkStrategy { new UDFCheckStrategy(stagingurl.path()) };
    if (!checkStrategy->check()) {
        fmWarning() << "Check Failed: " << checkStrategy->lastError();
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
    fmInfo() << "Burn UDF ret: " << isSuccess << manager->lastError() << localPath;
    delete manager;
}

void BurnUDFFilesJob::work()
{
    fmInfo() << "Start burn UDF files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;
    if (!fileSystemLimitsValid())
        return;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    fmInfo() << "End burn UDF files: " << curDev;
}

void BurnUDFFilesJob::finishFunc(bool verify, bool verifyRet)
{
    // 检查错误消息，处理特定的 UDF 刻录错误
    if (lastStatus == JobStatus::kFailed) {
        for (const QString &msg : lastSrcMessages) {
            if (msg.contains("free size is not enough to burn files", Qt::CaseInsensitive)) {
                lastError = tr("Not enough free space on the disc");
                lastSrcMessages.prepend(tr("The remaining space on the current disc is insufficient "
                                           "to hold the files to be burned. Please reduce the number "
                                           "of files and try again."));
                break;
            }
        }
    }

    // 调用父类的 finishFunc 处理通用逻辑
    AbstractBurnJob::finishFunc(verify, verifyRet);
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
    fmInfo() << "Dump ISO ret: " << isSuccess << manager->lastError() << imagePath;
    delete manager;
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
    fmInfo() << "Start dump ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageDump;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    fmInfo() << "End dump ISO image: " << curDev;
}
