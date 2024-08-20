// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   // cmake

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <dtkcore_config.h>
#include <dfm-base/utils/loggerrules.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>

Q_LOGGING_CATEGORY(logAppDaemon, "org.deepin.dde.filemanager.daemon")

static constexpr char kDaemonInterface[] { "org.deepin.plugin.daemon" };
static constexpr char kPluginCore[] { "daemonplugin-core" };
static constexpr char kLibCore[] { "libdaemonplugin-core.so" };

DFMBASE_USE_NAMESPACE

static void handleSIGTERM(int sig)
{
    qCCritical(logAppDaemon) << "daemon break with !SIGTERM! " << sig;

    if (qApp) {
        qApp->quit();
    }
}

[[noreturn]] static void handleSIGBUS(int sig)
{
    qCCritical(logAppDaemon) << "daemon break with !SIGBUS! " << sig;

    ::_Exit(EXIT_FAILURE);
}

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
#ifdef DTKCORE_CLASS_DConfigFile
    LoggerRules::instance().initLoggerRules();
#endif
    dpfLogManager->applySuggestedLogSettings();
}

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppDaemon) << "Load plugins but dconfig failed: " << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppDaemon) << QString("Load plugins path : %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/daemon");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_DAEMON_EDGE_DIR);
#endif
    QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "daemon.blackList").toStringList() };
#ifdef DISABLE_ANYTHING
    if (!blackNames.contains("daemonplugin-anything"))
        blackNames << "daemonplugin-anything";
#endif

    qCInfo(logAppDaemon) << "Using plugins dir:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::initialize({ kDaemonInterface }, pluginsDirs, blackNames);

    qCInfo(logAppDaemon) << "Depend library paths:" << QCoreApplication::libraryPaths();
    qCInfo(logAppDaemon) << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore)) {
        qCWarning(logAppDaemon) << corePlugin->fileName() << "is not" << kLibCore;
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
    initLog();

    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");

    DPF_NAMESPACE::backtrace::installStackTraceHandler();

    if (!pluginsLoad()) {
        qCCritical(logAppDaemon) << "Load plugin failed!";
        abort();
    }
    signal(SIGTERM, handleSIGTERM);
    signal(SIGBUS, handleSIGBUS);

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
