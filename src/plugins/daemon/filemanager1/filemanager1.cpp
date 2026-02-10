// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemanager1.h"
#include "filemanager1dbus.h"
#include "filemanager1adaptor.h"

#include <QDBusConnection>

DAEMONPFILEMANAGER1_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(DAEMONPFILEMANAGER1_NAMESPACE)

void FileManager1DBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    auto conn { QDBusConnection::sessionBus() };
    if (!conn.registerService("org.freedesktop.FileManager1")) {
        fmCritical() << "[FileManager1DBusWorker] Failed to register DBus service 'org.freedesktop.FileManager1'";
        return;
    }

    fmInfo() << "[FileManager1DBusWorker] Initializing FileManager1 DBus service";
    filemanager1.reset(new FileManager1DBus);
    Q_UNUSED(new FileManager1Adaptor(filemanager1.data()));
    if (!conn.registerObject("/org/freedesktop/FileManager1",
                             filemanager1.data())) {
        fmCritical() << "[FileManager1DBusWorker] Failed to register DBus object '/org/freedesktop/FileManager1'";
        filemanager1.reset(nullptr);
        return;
    }

    fmInfo() << "[FileManager1DBusWorker] FileManager1 DBus service initialized successfully";
}

void FileManager1::initialize()
{
    fmInfo() << "[FileManager1] Initializing FileManager1 plugin";
    FileManager1DBusWorker *worker { new FileManager1DBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &FileManager1::requestLaunch, worker, &FileManager1DBusWorker::launchService);
    workerThread.start();
}

bool FileManager1::start()
{
    fmInfo() << "[FileManager1] Starting FileManager1 service";
    emit requestLaunch();
    return true;
}

void FileManager1::stop()
{
    fmInfo() << "[FileManager1] Stopping FileManager1 service";
    workerThread.quit();
    workerThread.wait();
}

DAEMONPFILEMANAGER1_END_NAMESPACE
