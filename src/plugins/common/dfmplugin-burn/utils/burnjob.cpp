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

#include "dfm-base/base/device/devicecontroller.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <QDebug>
#include <QThread>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFM_BURN_USE_NS
using namespace GlobalServerDefines;

namespace GlobalPrivate {
static constexpr int kPipeBufferSize { 4096 };
static constexpr char kMapKeyPhase[] { "phase" };
static constexpr char kMapKeyStatus[] { "status" };
static constexpr char kMapKeySpeed[] { "speed" };
static constexpr char kMapKeyMsg[] { "msg" };
}   // namespace GlobalPrivate

BurnJob::BurnJob(QObject *parent)
    : QObject(parent)
{
    initConnect();
}

void BurnJob::initConnect()
{
    connect(&DeviceManagerInstance, &DeviceManager::blockDevicePropertyChanged,
            this, [](const QString &deviceId, const QString &property, const QVariant &val) {
                // TODO(zhangs): mediaChangeDetected
            });
}

bool BurnJob::prepare(const QString &devId)
{
    QVariantMap &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(devId);
    if (map.isEmpty()) {
        qWarning() << "Device info is empty";
        return false;
    }

    bool blank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };
    if (blank) {
        // TODO(zhangs): jump to computer
    } else {
        QString mpt { qvariant_cast<QString>(map[DeviceProperty::kMountPoint]) };
        if (!mpt.isEmpty()) {
            if (!DeviceController::instance()->unmountBlockDevice(devId)) {
                emit reqShowErrorMessage(tr("The device was not safely unmounted"), tr("Disk is busy, cannot unmount now"));
                return false;
            }
        }
    }
    return true;
}

void BurnJob::doOpticalDiskBlank(const QString &dev, const JobHandlePointer handler)
{
    QString devId { DeviceManager::blockDeviceId(dev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };

    qInfo() << "Start erase device: " << dev << devId;
    // TODO(zhangs): check unmount
    OpticalDiscManager *manager = new OpticalDiscManager(dev, this);
    connect(manager, &OpticalDiscManager::jobStatusChanged, this, &BurnJob::onJobUpdated);
    if (!manager->erase())
        qWarning() << "Erase Failed: " << manager->lastError();
    qInfo() << "End erase device: " << dev << devId;

    // TODO(zhangs): must show %100

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));
    emit handler->finishedNotify(info);

    // TODO(zhangs): rescan
}
void BurnJob::doUDFDataBurn(const QString &dev, const QUrl &url, const BurnJob::BurnConfig &conf, const JobHandlePointer handler)
{
    QString devId { DeviceManager::blockDeviceId(dev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };

    qInfo() << "Start burn UDF files: " << dev << devId;
    QString localPath { url.toLocalFile() };
    auto burnFunc = [=](int progressFd, int badFd) {
        OpticalDiscManager *manager = new OpticalDiscManager(dev, this);
        manager->setStageFile(localPath);
        manager->commit(conf.opts, conf.speeds, conf.volName); };
    auto readFunc = [=](int progressFd, int badFd) {
        // TODO(zhangs): impl me!
    };
    workInProcess(burnFunc, readFunc);

    qInfo() << "End burn UDF files: " << dev << devId;

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));
    emit handler->finishedNotify(info);
}

void BurnJob::doISODataBurn(const QString &dev, const QUrl &url, const BurnJob::BurnConfig &conf, const JobHandlePointer handler)
{
    QString devId { DeviceManager::blockDeviceId(dev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };

    qInfo() << "Start burn ISO files: " << dev << devId;
    if (!prepare(devId))
        return;
    QString localPath { url.toLocalFile() };
    auto burnFunc = [=](int progressFd, int badFd) {
        OpticalDiscManager *manager = new OpticalDiscManager(dev, this);
        manager->setStageFile(localPath);
        manager->commit(conf.opts, conf.speeds, conf.volName); };
    auto readFunc = [=](int progressFd, int badFd) {
        // TODO(zhangs): impl me!
    };
    workInProcess(burnFunc, readFunc);
    qInfo() << "End burn ISO files: " << dev << devId;

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));
    emit handler->finishedNotify(info);
}

void BurnJob::doISOImageBurn(const QString &dev, const QUrl &imageUrl, const BurnJob::BurnConfig &conf, const JobHandlePointer handler)
{
    QString devId { DeviceManager::blockDeviceId(dev) };
    JobInfoPointer info { new QMap<quint8, QVariant> };

    qInfo() << "Start burn ISO image: " << dev << devId;
    QString imgPath { imageUrl.toLocalFile() };
    auto burnFunc = [=](int progressFd, int badFd) {
        OpticalDiscManager *manager = new OpticalDiscManager(dev, this);
        manager->writeISO(imgPath, conf.speeds);
    };
    auto readFunc = [=](int progressFd, int badFd) {
        // TODO(zhangs): impl me!
    };
    workInProcess(burnFunc, readFunc);
    qInfo() << "End burn ISO image: " << dev << devId;

    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handler));
    emit handler->finishedNotify(info);
}

void BurnJob::onJobUpdated(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    // TODO(zhangs): imple me!
    qDebug() << "--->" << int(status) << progress << speed << message;
}

void BurnJob::workInProcess(const WorkFunc &writeFunc, const WorkFunc &readFunc)
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
