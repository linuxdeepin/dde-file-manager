// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdaemon.h"

#include "recentmanagerdbus.h"
#include "recentmanageradaptor.h"
#include "recentmanager.h"

#include <QDBusConnection>

SERVERRECENTMANAGER_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(SERVERRECENTMANAGER_NAMESPACE)

void RecentDaemon::initialize()
{
    fmInfo() << "[RecentDaemon::initialize] Initializing recent daemon service";
    qRegisterMetaType<SERVERRECENTMANAGER_NAMESPACE::RecentItem>("RecentItem");
    RecentManager::instance().initialize();
    fmInfo() << "[RecentDaemon::initialize] Recent daemon service initialized successfully";
}

bool RecentDaemon::start()
{
    fmInfo() << "[RecentDaemon::start] Starting recent daemon DBus service";
    auto conn = QDBusConnection::sessionBus();
    
    // register service
    if (!conn.registerService("org.deepin.Filemanager.Daemon")) {
        fmCritical() << "[RecentDaemon::start] Failed to register DBus service 'org.deepin.Filemanager.Daemon'";
        ::exit(EXIT_FAILURE);
    }
    fmInfo() << "[RecentDaemon::start] DBus service 'org.deepin.Filemanager.Daemon' registered successfully";

    recentManager.reset(new RecentManagerDBus);
    Q_UNUSED(new RecentManagerAdaptor(recentManager.data()));
    if (!conn.registerObject("/org/deepin/Filemanager/Daemon/RecentManager",
                             recentManager.data())) {
        fmCritical() << "[RecentDaemon::start] Failed to register DBus object '/org/deepin/Filemanager/Daemon/RecentManager'";
        recentManager.reset(nullptr);
        return false;
    }
    fmInfo() << "[RecentDaemon::start] DBus object '/org/deepin/Filemanager/Daemon/RecentManager' registered successfully";

    // start watch xbel path
    RecentManager::instance().startWatch();
    fmInfo() << "[RecentDaemon::start] Recent daemon service started successfully";

    return true;
}

void RecentDaemon::stop()
{
    fmInfo() << "[RecentDaemon::stop] Stopping recent daemon service";
    RecentManager::instance().finalize();
    fmInfo() << "[RecentDaemon::stop] Recent daemon service stopped successfully";
}

SERVERRECENTMANAGER_END_NAMESPACE
