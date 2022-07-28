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

#include <dfm-framework/dpf.h>

#include <iostream>
#include <algorithm>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

#define BUILD_VERSION ((QString(VERSION) == "") ? "6.0.0.0" : QString(VERSION))

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const kFmPluginInterface = "org.deepin.plugin.desktop";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
static const char *const kPluginCore = "ddplugin-core";
static const char *const kLibCore = "libddplugin-core.so";

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    // set plugin iid from qt style
    DPF_NAMESPACE::LifeCycle::addPluginIID(kFmPluginInterface);
    DPF_NAMESPACE::LifeCycle::addPluginIID(kCommonPluginInterface);

    QString pluginsDir(qApp->applicationDirPath() + "/../../plugins");
    QStringList pluginsDirs;
    if (!QDir(pluginsDir).exists()) {
        qInfo() << QString("Path does not exist, use path : %1").arg(DFM_PLUGIN_COMMON_CORE_DIR);
        pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                    << QString(DFM_PLUGIN_DESKTOP_CORE_DIR)
                    << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                    << QString(DFM_PLUGIN_DESKTOP_EDGE_DIR);
    } else {
        pluginsDirs.push_back(pluginsDir + "/desktop");
        pluginsDirs.push_back(pluginsDir + "/common");
        pluginsDirs.push_back(pluginsDir);
    }
    qDebug() << "using plugins dir:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::setPluginPaths(pluginsDirs);

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore)) {
        qWarning() << corePlugin->fileName() << "is not" << kLibCore;
        return false;
    }
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
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
    dpfLogManager->setLogFormat(logFormat);
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

constexpr char kDesktopServiceName[] { "com.deepin.dde.desktop" };
constexpr char kDesktopServicePath[] { "/com/deepin/dde/desktop" };
constexpr char kDesktopServiceInterface[] { "com.deepin.dde.desktop" };

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setApplicationDisplayName(a.translate("DesktopMain", "Desktop"));
    a.setApplicationVersion(BUILD_VERSION);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    DPF_NAMESPACE::backtrace::initbacktrace();
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

    {
        // load translation
        QString appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
