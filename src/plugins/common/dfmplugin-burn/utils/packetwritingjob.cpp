// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packetwritingjob.h"

#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/finallyutil.h>

DPBURN_BEGIN_NAMESPACE
DFM_BURN_USE_NS
DFMBASE_USE_NAMESPACE

PacketWritingScheduler &PacketWritingScheduler::instance()
{
    static PacketWritingScheduler ins;
    return ins;
}

void PacketWritingScheduler::addJob(AbstractPacketWritingJob *job)
{
    Q_ASSERT(job);
    jobs.enqueue(job);
    if (!rotationTimer.isActive())
        rotationTimer.start(3000);
}

void PacketWritingScheduler::onTimeout()
{
    if (jobs.isEmpty()) {
        fmInfo() << "All packet writing jobs fnished, timer will stop work";
        rotationTimer.stop();
        return;
    }

    auto job { jobs.head() };
    if (job->isRunning()) {
        fmDebug() << "Previous job " << job << "is working, wait again...";
        return;
    }

    connect(job, &QThread::finished, this, [this, job]() {
        auto dequeJob { jobs.dequeue() };
        Q_ASSERT(dequeJob == job);
        fmDebug() << "Job working fnieshed: " << dequeJob;
        dequeJob->deleteLater();
    });

    fmDebug() << "A packet writing job start: " << job;
    job->start();
}

PacketWritingScheduler::PacketWritingScheduler(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    connect(&rotationTimer, &QTimer::timeout, this, &PacketWritingScheduler::onTimeout);
}

AbstractPacketWritingJob::AbstractPacketWritingJob(const QString &device, QObject *parent)
    : QThread(parent), curDevice(device)
{
}

AbstractPacketWritingJob::~AbstractPacketWritingJob()
{
}

QString AbstractPacketWritingJob::device() const
{
    return curDevice;
}

void AbstractPacketWritingJob::run()
{
    Q_ASSERT(!curDevice.isEmpty());
    const QString &mnt { DeviceUtils::getMountInfo(curDevice) };
    if (mnt.isEmpty()) {
        fmWarning() << "Packet wring mount point is empty";
        // TODO: error dialog
        return;
    }

    // REFACTOR: fork?
    fmInfo() << "Start packet writing for device: " << curDevice;
    pwController.reset(new DPacketWritingController { curDevice, mnt });
    FinallyUtil finally { [this]() {
        pwController->close();
        fmInfo() << "Ended packet writing" << pwController->lastError().isEmpty();
    } };

    if (!pwController->open()) {
        fmWarning() << "Open packet writing device failed: "
                    << pwController->lastError();
        // TODO: error dialog
        return;
    }

    if (!work()) {
        fmWarning() << "Packet writing failed: " << pwController->lastError();
        // TODO: error dialog
    }
}

QStringList AbstractPacketWritingJob::urls2Names(const QList<QUrl> &urls) const
{
    QStringList names;
    std::transform(urls.begin(), urls.end(), std::back_inserter(names),
                   [](const QUrl &url) {
                       return url.fileName();
                   });
    return names;
}

PutPacketWritingJob::PutPacketWritingJob(const QString &device, QObject *parent)
    : AbstractPacketWritingJob(device, parent)
{
}

QList<QUrl> PutPacketWritingJob::getPendingUrls() const
{
    return pendingUrls;
}

void PutPacketWritingJob::setPendingUrls(const QList<QUrl> &value)
{
    pendingUrls = value;
}

bool PutPacketWritingJob::work()
{
    const auto &names { urls2Names(pendingUrls) };
    fmInfo() << "Start put: " << names;
    if (names.isEmpty())
        return false;
    return std::all_of(names.begin(), names.end(), [this](const QString &name) {
        return pwController && pwController->put(name);
    });
}

RemovePacketWritingJob::RemovePacketWritingJob(const QString &device, QObject *parent)
    : AbstractPacketWritingJob(device, parent)
{
}

QList<QUrl> RemovePacketWritingJob::getPendingUrls() const
{
    return pendingUrls;
}

void RemovePacketWritingJob::setPendingUrls(const QList<QUrl> &value)
{
    pendingUrls = value;
}

bool RemovePacketWritingJob::work()
{
    const auto &names { urls2Names(pendingUrls) };
    fmInfo() << "Start remove: " << names;
    return std::all_of(names.begin(), names.end(), [this](const QString &name) {
        return pwController && pwController->rm(name);
    });
}

RenamePacketWritingJob::RenamePacketWritingJob(const QString &device, QObject *parent)
    : AbstractPacketWritingJob(device, parent)

{
}

QUrl RenamePacketWritingJob::getSrcUrl() const
{
    return srcUrl;
}

void RenamePacketWritingJob::setSrcUrl(const QUrl &value)
{
    srcUrl = value;
}

QUrl RenamePacketWritingJob::getDestUrl() const
{
    return destUrl;
}

void RenamePacketWritingJob::setDestUrl(const QUrl &value)
{
    destUrl = value;
}

bool RenamePacketWritingJob::work()
{
    QString srcName { urls2Names({ srcUrl }).at(0) };
    QString destName { urls2Names({ destUrl }).at(0) };

    fmInfo() << "Start rename " << srcName << "to" << destName;
    return pwController && pwController->mv(srcName, destName);
}

DPBURN_END_NAMESPACE
