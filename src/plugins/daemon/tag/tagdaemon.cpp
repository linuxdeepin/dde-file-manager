// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdaemon.h"
#include "tagmanagerdbus.h"
#include "tagmanageradaptor.h"

#include <QDBusConnection>

DAEMONPTAG_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(DAEMONPTAG_NAMESPACE)

static constexpr char kTagManagerObjPath[] { "/org/deepin/Filemanager/Daemon/TagManager" };

void TagDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    auto conn { QDBusConnection::sessionBus() };

    fmInfo() << "TagDBusWorker::launchService: Initializing DBus TagManager service";
    
    tagManager.reset(new TagManagerDBus);
    Q_UNUSED(new TagManagerAdaptor(tagManager.data()));
    
    if (!conn.registerObject(kTagManagerObjPath, tagManager.data())) {
        fmCritical() << "TagDBusWorker::launchService: Failed to register DBus object at path:" << kTagManagerObjPath;
        tagManager.reset(nullptr);
        return;
    }

    fmInfo() << "TagDBusWorker::launchService: DBus object registered successfully at path:" << kTagManagerObjPath;
    
    // Emit service ready signal
    tagManager->TagsServiceReady();
    fmInfo() << "TagDBusWorker::launchService: TagManager service initialized and ready";
}

void TagDaemon::initialize()
{
    fmInfo() << "TagDaemon::initialize: Initializing tag daemon service";
    
    TagDBusWorker *worker { new TagDBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &TagDaemon::requestLaunch, worker, &TagDBusWorker::launchService);
    workerThread.start();
    
    fmInfo() << "TagDaemon::initialize: Tag daemon service initialized successfully";
}

bool TagDaemon::start()
{
    fmInfo() << "TagDaemon::start: Starting tag daemon service";
    emit requestLaunch();
    fmInfo() << "TagDaemon::start: Tag daemon service start request sent";
    return true;
}

void TagDaemon::stop()
{
    fmInfo() << "TagDaemon::stop: Stopping tag daemon service";
    workerThread.quit();
    workerThread.wait();
    fmInfo() << "TagDaemon::stop: Tag daemon service stopped successfully";
}

DAEMONPTAG_END_NAMESPACE
