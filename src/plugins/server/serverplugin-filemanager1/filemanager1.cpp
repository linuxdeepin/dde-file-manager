// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemanager1.h"
#include "filemanager1dbus.h"
#include "filemanager1adaptor.h"

#include <QDBusConnection>

SERVERFILEMANAGER1_BEGIN_NAMESPACE

void FileManager1DBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    auto conn { QDBusConnection::sessionBus() };
    if (!conn.registerService("org.freedesktop.FileManager1")) {
        qWarning("Cannot register the \"org.freedesktop.FileManager1\" service.\n");
        return;
    }

    qInfo() << "Init DBus FileManager1 start";
    filemanager1.reset(new FileManager1DBus);
    Q_UNUSED(new FileManager1Adaptor(filemanager1.data()));
    if (!conn.registerObject("/org/freedesktop/FileManager1",
                             filemanager1.data())) {
        qWarning("Cannot register the \"/org/freedesktop/FileManager1\" object.\n");
        filemanager1.reset(nullptr);
    }

    qInfo() << "Init DBus FileManager1 end";
}

void FileManager1::initialize()
{
    FileManager1DBusWorker *worker { new FileManager1DBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &FileManager1::requestLaunch, worker, &FileManager1DBusWorker::launchService);
    workerThread.start();
}

bool FileManager1::start()
{
    emit requestLaunch();
    return true;
}

void FileManager1::stop()
{
    workerThread.quit();
    workerThread.wait();
}

SERVERFILEMANAGER1_END_NAMESPACE
