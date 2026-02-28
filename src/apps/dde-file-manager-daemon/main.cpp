// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QTimer>
#include <QSocketNotifier>

#include <signal.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(logAppDaemon, "org.deepin.dde.Filemanager.Daemon")

static constexpr char kDaemonInterface[] { "org.deepin.plugin.daemon" };
static constexpr char kPluginCore[] { "dfmdaemon-core-plugin" };
static constexpr char kLibCore[] { "libdfmdaemon-core-plugin.so" };

// Self-pipe trick: fd[0]=read end (QSocketNotifier), fd[1]=write end (signal handler)
static int g_sigTermPipe[2] { -1, -1 };

DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

static void initLogFilter()
{
#ifdef DTKCORE_CLASS_DConfigFile
    LoggerRules::instance().initLoggerRules();
#endif
}

static void initLogSetting()
{
    dpfLogManager->applySuggestedLogSettings();
}

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppDaemon) << "pluginsLoad: Failed to load plugins dconfig:" << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppDaemon) << "pluginsLoad: Using debug plugins path:" << pluginsDir;
    pluginsDirs.push_back(pluginsDir + "/daemon");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_DAEMON_EDGE_DIR);
#endif
    qCInfo(logAppDaemon) << "pluginsLoad: Using plugins directories:" << pluginsDirs;
    QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "daemon.blackList").toStringList() };
    qCDebug(logAppDaemon) << "pluginsLoad: Blacklisted plugins:" << blackNames;
    DPF_NAMESPACE::LifeCycle::initialize({ kDaemonInterface }, pluginsDirs, blackNames);

    qCInfo(logAppDaemon) << "pluginsLoad: Library paths:" << QCoreApplication::libraryPaths();
    qCInfo(logAppDaemon) << "pluginsLoad: Plugin paths:" << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins()) {
        qCCritical(logAppDaemon) << "pluginsLoad: Failed to read plugins";
        return false;
    }

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull()) {
        qCCritical(logAppDaemon) << "pluginsLoad: Core plugin not found:" << kPluginCore;
        return false;
    }
    if (!corePlugin->fileName().contains(kLibCore)) {
        qCCritical(logAppDaemon) << "pluginsLoad: Core plugin library mismatch, expected:" << kLibCore
                                 << "actual:" << corePlugin->fileName();
        return false;
    }
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin)) {
        qCCritical(logAppDaemon) << "pluginsLoad: Failed to load core plugin:" << kPluginCore;
        return false;
    }

    qCInfo(logAppDaemon) << "pluginsLoad: Core plugin loaded successfully";

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins()) {
        qCCritical(logAppDaemon) << "pluginsLoad: Failed to load remaining plugins";
        return false;
    }

    qCInfo(logAppDaemon) << "pluginsLoad: All plugins loaded successfully";
    return true;
}

static void handleSIGTERM(int /*sig*/)
{
    // Only async-signal-safe operations are allowed here.
    // write() is async-signal-safe; qApp->quit() is NOT, so we use self-pipe trick
    // to delegate the actual quit() call to the main event loop via QSocketNotifier.
    const char byte = 1;
    (void)::write(g_sigTermPipe[1], &byte, sizeof(byte));
}

[[noreturn]] static void handleSIGABRT(int sig)
{
    qCCritical(logAppDaemon) << "handleSIGABRT: Received SIGABRT signal:" << sig;
    // WORKAROUND: cannot receive SIGTERM when shutdown or reboot
    // see: bug-228373
    ::_exit(1);
}

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    initLogFilter();
    DApplication a(argc, argv);

    // BUG-278055
    initLogSetting();

    a.setOrganizationName(ORGANIZATION_NAME);
    {
        // load translation
        QString appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
    }

    qCInfo(logAppDaemon) << "main: File manager daemon started, version:" << a.applicationVersion();

    // Set up self-pipe so the signal handler can safely wake the main event loop
    if (::pipe(g_sigTermPipe) != 0) {
        qCWarning(logAppDaemon) << "main: Failed to create SIGTERM self-pipe";
    } else {
        auto *sigTermNotifier = new QSocketNotifier(g_sigTermPipe[0], QSocketNotifier::Read, &a);
        QObject::connect(sigTermNotifier, &QSocketNotifier::activated, &a, [&a]() {
            char tmp;
            (void)::read(g_sigTermPipe[0], &tmp, sizeof(tmp));
            qCInfo(logAppDaemon) << "main: SIGTERM received via self-pipe, quitting main event loop";
            a.quit();
        });
    }
    signal(SIGTERM, handleSIGTERM);
    signal(SIGABRT, handleSIGABRT);

    DPF_NAMESPACE::backtrace::installStackTraceHandler();

    if (!pluginsLoad()) {
        qCCritical(logAppDaemon) << "main: Failed to load plugins, terminating daemon";
        Q_ASSERT_X(false, "pluginsLoad", "Failed to load plugins");
    }

    qCInfo(logAppDaemon) << "main: Daemon initialization completed successfully";
    int ret { a.exec() };

    // Close self-pipe fds to release kernel resources
    if (g_sigTermPipe[0] != -1) {
        ::close(g_sigTermPipe[0]);
        ::close(g_sigTermPipe[1]);
        g_sigTermPipe[0] = g_sigTermPipe[1] = -1;
    }

    qCInfo(logAppDaemon) << "main: Shutting down plugins";
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();

    return ret;
}
