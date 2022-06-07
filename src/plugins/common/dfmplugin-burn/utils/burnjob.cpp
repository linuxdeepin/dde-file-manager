/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#include "burnjob.h"
#include "utils/burnhelper.h"
#include "events/burneventcaller.h"

#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/decorator/decoratorfileoperator.h"

#include <QDebug>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

DPBURN_USE_NAMESPACE
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
}

void AbstractBurnJob::setProperty(AbstractBurnJob::PropertyType type, const QVariant &val)
{
    curProperty[type] = val;
}

void AbstractBurnJob::updateMessage(JobInfoPointer ptr)
{
    Q_ASSERT(ptr);
    if (curJobType != JobType::kOpticalBlank) {
        QString msg1 = tr("Burning disc %1, please wait...").arg(curDev);
        QString msg2 = tr("Writing data...");
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, msg1);
        if (curJobType == JobType::kOpticalCheck)
            msg2 = tr("Verifying data...");
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, msg2);
        emit jobHandlePtr->currentTaskNotify(ptr);
    }
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

    comfort();

    // check
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };
    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    double bad {};
    if (check && lastStatus != JobStatus::kFailed)
        read(checkFd, &bad, sizeof(bad));
    bool checkRet { !(check && (bad > (2 + 1e-6))) };

    // show result dialog
    if (lastStatus == JobStatus::kFailed) {
        jobSuccess = false;
        if (check && checkRet)
            emit requestCompletionDialog(tr("Data verification successful."), "dialog-ok");
        else
            emit requestFailureDialog(static_cast<int>(curJobType), lastError, lastSrcMessages);
    } else {
        jobSuccess = true;
        if (check)
            emit requestCompletionDialog(tr("Data verification successful."), "dialog-ok");
        else
            emit requestCompletionDialog(tr("Burn process completed"), "dialog-ok");
    }

    DeviceManager::instance()->ejectBlockDev(curDevId);
}

void AbstractBurnJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(progressFd)
    Q_UNUSED(checkFd)
}

void AbstractBurnJob::run()
{
    curDevId = { DeviceUtils::getBlockDeviceId(curDev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };

    work();

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(jobHandlePtr));
    emit jobHandlePtr->finishedNotify(info);
}

bool AbstractBurnJob::readyToWork()
{
    QVariantMap &&map = DevProxyMng->queryBlockInfo(curDevId);
    if (map.isEmpty()) {
        qWarning() << "Device info is empty";
        return false;
    }

    bool blank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    // for dvd+rw disc, erase operation only overwrite some blocks which used to present filesystem,
    // so the blank field is still false even if it can be write datas from the beginning,
    if (static_cast<MediaType>(map[DeviceProperty::kOpticalMediaType].toUInt()) == MediaType::kDVD_PLUS_RW)
        blank |= map[DeviceProperty::kSizeTotal].toULongLong() == map[DeviceProperty::kSizeFree].toULongLong();

    if (blank) {
        QString tag = curDevId.mid(curDevId.lastIndexOf("/") + 1);
        QUrl url(QString("burn:///dev/%1/disc_files/").arg(tag));
        BurnEventCaller::sendCloseTab(url);
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
    connect(manager, &DOpticalDiscManager::jobStatusChanged, this,
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

void AbstractBurnJob::deleteStagingFiles()
{
    QUrl url { curProperty[PropertyType::KStagingUrl].toUrl() };
    if (!DecoratorFileOperator(url).deleteFile())
        qWarning() << "Delete " << url << "failed!";
    else
        qInfo() << "Delete cache folder: " << url << "success";
}

void AbstractBurnJob::onJobUpdated(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    lastStatus = status;

    Q_ASSERT(jobHandlePtr);
    JobInfoPointer info { new QMap<quint8, QVariant> };

    // hide btn
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateHideKey, true);
    emit jobHandlePtr->stateChangedNotify(info);

    // update progress
    if (progress > 0 && progress <= 100 && progress > lastProgress) {
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
    if (status == JobStatus::kRunning)
        info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, speed);
    else
        info->insert(AbstractJobHandler::NotifyInfoKey::kSpeedKey, "");
    emit jobHandlePtr->speedUpdatedNotify(info);
}

EraseJob::EraseJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
}

void EraseJob::updateMessage(JobInfoPointer ptr)
{
    Q_ASSERT(ptr);
    if (curJobType == JobType::kOpticalBlank) {
        QString msg = tr("Erasing disc %1, please wait...").arg(curDev);
        ptr->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, msg);
        emit jobHandlePtr->currentTaskNotify(ptr);
    }
}

void EraseJob::work()
{
    qInfo() << "Start erase device: " << curDev;

    // TODO(zhangs): check unmount
    curJobType = JobType::kOpticalBlank;
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
    DevMngIns->mountBlockDevAsync(curDevId, {}, [this](bool, DFMMOUNT::DeviceError, const QString &) {
        DevProxyMng->reloadOpticalInfo(curDevId);
    });
}

BurnISOFilesJob::BurnISOFilesJob(const QString &dev, const JobHandlePointer handler)
    : AbstractBurnJob(dev, handler)
{
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
    curJobType = JobType::kOpticalBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    if (jobSuccess)
        deleteStagingFiles();
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
    curJobType = JobType::kOpticalImageBurn;
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
    curJobType = JobType::kOpticalBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    if (jobSuccess)
        deleteStagingFiles();
    qInfo() << "End burn UDF files: " << curDev;
}
