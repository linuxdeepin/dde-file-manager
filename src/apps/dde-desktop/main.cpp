/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: huanyu<huanyu@uniontech.com>
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

#include "config.h"   //cmake

#include <dfm-framework/framework.h>

#include <DApplication>
#include <DMainWindow>

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QtGlobal>
#include <QDBusInterface>

#include <iostream>
#include <algorithm>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const kFmPluginInterface = "org.deepin.plugin.desktop";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
static const char *const kPluginCore = "ddplugin-core";
static const char *const kLibCore = "libddplugin-core.so";

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
    lifeCycle.addPluginIID(kFmPluginInterface);
    lifeCycle.addPluginIID(kCommonPluginInterface);

    QDir dir(qApp->applicationDirPath());
    QString pluginsDir;
    if (!dir.cd("../../plugins/")) {
        qInfo() << QString("Path does not exist, use path : %1").arg(DFM_PLUGIN_PATH);
        pluginsDir = DFM_PLUGIN_PATH;
    } else {
        pluginsDir = dir.absolutePath();
    }
    qDebug() << "using plugins dir:" << pluginsDir;

    lifeCycle.setPluginPaths({ pluginsDir });

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!lifeCycle.readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = lifeCycle.pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore)) {
        qWarning() << corePlugin->fileName() << "is not" << kLibCore;
        return false;
    }
    if (!lifeCycle.loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!lifeCycle.loadPlugins())
        return false;

    dpfCheckTimeEnd();

    return true;
}

static void registerDDESession()
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    QByteArray cookie = qgetenv(envName);
    qunsetenv(envName);

    if (!cookie.isEmpty()) {
        QDBusInterface iface("com.deepin.SessionManager",
                             "/com/deepin/SessionManager",
                             "com.deepin.SessionManager",
                             QDBusConnection::sessionBus());
        iface.call("Register", QString(cookie));
    }
}

static void initLog()
{
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    dpfInstance.log().setLogFormat(logFormat);
    dpfInstance.log().registerConsoleAppender();
    dpfInstance.log().registerFileAppender();
}

constexpr char kDesktopServiceName[] { "com.deepin.dde.desktop" };
constexpr char kDesktopServicePath[] { "/com/deepin/dde/desktop" };
constexpr char kDesktopServiceInterface[] { "com.deepin.dde.desktop" };

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setApplicationDisplayName(a.translate("DesktopMain", "Desktop"));
    //a.setApplicationVersion(DApplication::buildVersion((GIT_VERSION))); //todo(zs)
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    dpfInstance.initialize();
    initLog();

    // Notify dde-desktop start up
    // if (!fileDialogOnly)
    if (true) {
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService(kDesktopServiceName)) {
            qCritical() << "registerService Failed, maybe service exist" << conn.lastError();
            exit(0x0002);
        }

        //        auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
        //        if (!conn.registerObject(DesktopServicePath, Desktop::instance(), registerOptions)) {
        //            qCritical() << "registerObject Failed" << conn.lastError();
        //            exit(0x0003);
        //        }

        registerDDESession();
    }

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    return a.exec();
}
