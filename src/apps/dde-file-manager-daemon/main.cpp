// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/loggerrules.h>

#include <dfm-framework/dpf.h>

#include <DApplication>
#include <DSysInfo>

#include <QDebug>
#include <QDir>

#include <signal.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(logAppDaemon, "org.deepin.dde.Filemanager.Daemon")

static constexpr char kDaemonInterface[] { "org.deepin.plugin.daemon" };
static constexpr char kPluginCore[] { "daemonplugin-core" };
static constexpr char kLibCore[] { "libdaemonplugin-core.so" };

DFMBASE_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
    qCInfo(logAppDaemon) << "Using plugins dir:" << pluginsDirs;
    QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "server.blackList").toStringList() };
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

static void handleSIGTERM(int sig)
{
    qCCritical(logAppDaemon) << "break with !SIGTERM! " << sig;

    if (qApp) {
        qApp->quit();
    }
}

[[noreturn]] static void handleSIGABRT(int sig)
{
    qCCritical(logAppDaemon) << "break with !SIGABRT! " << sig;
    // WORKAROUND: cannot receive SIGTERM when shutdown or reboot
    // see: bug-228373
    ::_exit(1);
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    initLog();
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    {
        // load translation
        QString appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
    }

    signal(SIGTERM, handleSIGTERM);
    signal(SIGABRT, handleSIGABRT);

    DPF_NAMESPACE::backtrace::installStackTraceHandler();

    if (!pluginsLoad()) {
        qCCritical(logAppDaemon) << "Load pugin failed!";
        abort();
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
