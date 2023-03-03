// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   // cmake

#include <QCoreApplication>

#include <QDebug>
#include <QDir>

#include <dfm-framework/dpf.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

static constexpr char kDaemonInterface[] { "org.deepin.plugin.daemon" };
static constexpr char kPluginCore[] { "daemonplugin-core" };
static constexpr char kLibCore[] { "libdaemonplugin-core.so" };

static void initEnv()
{
    ///###: why?
    ///###: when dbus invoke a daemon the variants in the environment of daemon(s) are empty.
    ///###: So we need to set them.
    if (!qEnvironmentVariableIsSet("LANG")) {
        qputenv("LANG", "en_US.UTF8");
    }

    if (!qEnvironmentVariableIsSet("LANGUAGE")) {
        qputenv("LANGUAGE", "en_US");
    }

    if (!qEnvironmentVariableIsSet("HOME")) {
        qputenv("HOME", getpwuid(getuid())->pw_dir);
    }
}

static void initLog()
{

    QString logPath = "/var/log/" + QCoreApplication::organizationName() + QLatin1Char('/') + QCoreApplication::applicationName() + QLatin1Char('/');
    QDir logDir(logPath);
    if (!logDir.exists())
        QDir().mkpath(logPath);

    dpfLogManager->setlogFilePath(logPath + QCoreApplication::applicationName() + ".log");
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

static bool pluginsLoad()
{
    QString pluginsDir(qApp->applicationDirPath() + "/../../plugins");
    QStringList pluginsDirs;
    if (!QDir(pluginsDir).exists()) {
        qInfo() << QString("Path does not exist, use path : %1").arg(DFM_PLUGIN_COMMON_CORE_DIR);
        pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                    << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                    << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                    << QString(DFM_PLUGIN_DAEMON_EDGE_DIR)
                    << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
    } else {
        pluginsDirs.push_back(pluginsDir + "/daemon");
        pluginsDirs.push_back(pluginsDir + "/common");
        pluginsDirs.push_back(pluginsDir);
    }

    qDebug() << "using plugins dir:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::initialize({ kDaemonInterface }, pluginsDirs);

    qInfo() << "Depend library paths:" << QCoreApplication::libraryPaths();
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

    return true;
}

int main(int argc, char *argv[])
{
    initEnv();
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");

    DPF_NAMESPACE::backtrace::installStackTraceHandler();

    initLog();
    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
