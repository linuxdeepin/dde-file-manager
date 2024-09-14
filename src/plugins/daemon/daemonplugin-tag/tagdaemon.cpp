// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdaemon.h"
#include "tagmanagerdbus.h"
#include "tagmanageradaptor.h"

#include <QDBusConnection>

DAEMONPTAG_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(DAEMONPTAG_NAMESPACE)

static constexpr char kTagManagerObjPath[] { "/org/deepin/Filemanager/Daemon/TagManager" };

void TagDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());
    auto conn { QDBusConnection::sessionBus() };

    fmInfo() << "Init DBus TagManager start";
    tagManager.reset(new TagManagerDBus);
    Q_UNUSED(new TagManagerAdaptor(tagManager.data()));
    if (!conn.registerObject(kTagManagerObjPath,
                             tagManager.data())) {
        fmWarning() << QString("Cannot register the \"%1\" object.\n").arg(kTagManagerObjPath);
        tagManager.reset(nullptr);
    } else {
        tagManager->TagsServiceReady();
    }
    fmInfo() << "Init DBus TagManager end";
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

DAEMONPTAG_END_NAMESPACE
