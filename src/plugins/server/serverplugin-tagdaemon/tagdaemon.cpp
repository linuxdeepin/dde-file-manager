// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdaemon.h"
#include "tagmanagerdbus.h"
#include "tagmanageradaptor.h"

#include <QDBusConnection>

SERVERTAGDAEMON_BEGIN_NAMESPACE

void TagDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    auto conn { QDBusConnection::sessionBus() };

    qInfo() << "Init DBus TagManager start";
    tagManager.reset(new TagManagerDBus);
    Q_UNUSED(new TagManagerAdaptor(tagManager.data()));
    if (!conn.registerObject("/org/deepin/filemanager/server/TagManager",
                             tagManager.data())) {
        qWarning("Cannot register the \"/org/deepin/filemanager/server/TagManager\" object.\n");
        tagManager.reset(nullptr);
    } else {
        tagManager->TagsServiceReady();
    }
    qInfo() << "Init DBus TagManager end";
}

void TagDaemon::initialize()
{
    TagDBusWorker *worker { new TagDBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &TagDaemon::requestLaunch, worker, &TagDBusWorker::launchService);
    workerThread.start();
}

bool TagDaemon::start()
{
    emit requestLaunch();
    return true;
}

void TagDaemon::stop()
{
    workerThread.quit();
    workerThread.wait();
}

SERVERTAGDAEMON_END_NAMESPACE
