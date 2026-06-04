// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalshare.h"
#include "opticalsharedbus.h"
#include "opticalshareadaptor.h"

#include <QDBusConnection>

DAEMONPOPTICALSHARE_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(DAEMONPOPTICALSHARE_NAMESPACE)

void OpticalShareDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    auto conn = QDBusConnection::sessionBus();
    if (!conn.registerService("org.deepin.Filemanager.OpticalShare")) {
        fmCritical() << "[OpticalShareDBusWorker] Failed to register D-Bus service 'org.deepin.Filemanager.OpticalShare'";
        return;
    }

    fmInfo() << "[OpticalShareDBusWorker] Initializing OpticalShare D-Bus service";
    opticalShare.reset(new OpticalShareDBus);
    Q_UNUSED(new OpticalShareAdaptor(opticalShare.data()));
    if (!conn.registerObject("/org/deepin/Filemanager/Daemon/OpticalShare",
                             opticalShare.data())) {
        fmCritical() << "[OpticalShareDBusWorker] Failed to register D-Bus object '/org/deepin/Filemanager/Daemon/OpticalShare'";
        opticalShare.reset(nullptr);
        return;
    }

    fmInfo() << "[OpticalShareDBusWorker] OpticalShare D-Bus service initialized successfully";
}

void OpticalShare::initialize()
{
    fmInfo() << "[OpticalShare] Initializing OpticalShare plugin";
    OpticalShareDBusWorker *worker = new OpticalShareDBusWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &OpticalShare::requestLaunch, worker, &OpticalShareDBusWorker::launchService);
    workerThread.start();
}

bool OpticalShare::start()
{
    fmInfo() << "[OpticalShare] Starting OpticalShare service";
    emit requestLaunch();
    return true;
}

void OpticalShare::stop()
{
    fmInfo() << "[OpticalShare] Stopping OpticalShare service";
    workerThread.quit();
    workerThread.wait();
}

DAEMONPOPTICALSHARE_END_NAMESPACE
