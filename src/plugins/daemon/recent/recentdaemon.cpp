// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdaemon.h"

#include "recentmanagerdbus.h"
#include "recentmanageradaptor.h"
#include "recentmanager.h"

#include <QDBusConnection>

SERVERRECENTMANAGER_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(SERVERRECENTMANAGER_NAMESPACE)

void RecentDaemon::initialize()
{
    qRegisterMetaType<SERVERRECENTMANAGER_NAMESPACE::RecentItem>("RecentItem");
    RecentManager::instance().initialize();
}

bool RecentDaemon::start()
{
    fmInfo() << "Initializing DBus RecentManager";
    auto conn = QDBusConnection::sessionBus();
    // register service
    if (!conn.registerService("org.deepin.Filemanager.Daemon")) {
        fmCritical("Cannot register the \"org.deepin.Filemanager.Daemon\" service!!!\n");
        ::exit(EXIT_FAILURE);
    }

    recentManager.reset(new RecentManagerDBus);
    Q_UNUSED(new RecentManagerAdaptor(recentManager.data()));
    if (!conn.registerObject("/org/deepin/Filemanager/Daemon/RecentManager",
                             recentManager.data())) {
        fmWarning() << "Cannot register the \"/org/deepin/Filemanager/Daemon/RecentManager\" object.";
        recentManager.reset(nullptr);
    }

    fmInfo() << "DBus RecentManager initialization completed";

    // start watch xbel path
    RecentManager::instance().startWatch();

    return true;
}

void RecentDaemon::stop()
{
    RecentManager::instance().finalize();
}

SERVERRECENTMANAGER_END_NAMESPACE
