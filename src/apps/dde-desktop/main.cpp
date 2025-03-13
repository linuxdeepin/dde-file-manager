// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopdbusinterface.h"

#include "config.h"   //cmake
#include "tools/upgrade/builtininterface.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/loggerrules.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/windowutils.h>

#include <dfm-framework/dpf.h>

#include <DApplication>
#include <DMainWindow>

#include <QMainWindow>
#include <QWidget>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QtGlobal>
#include <QDBusInterface>
#include <QProcess>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QDBusConnectionInterface>
#include <QSurfaceFormat>

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <malloc.h>

Q_LOGGING_CATEGORY(logAppDesktop, "org.deepin.dde.filemanager.desktop")

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dde_desktop;

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

#define BUILD_VERSION ((QString(VERSION) == "") ? "6.0.0.0" : QString(VERSION))

/// @brief PLUGIN_INTERFACE 默认插件iid
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    static const char *const kDesktopPluginInterface = "org.deepin.plugin.desktop";
    static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
    static const char *const kPluginCore = "ddplugin-core";
    static const char *const kLibCore = "libddplugin-core.so";
#else
    static const char *const kDesktopPluginInterface = "org.deepin.plugin.desktop.qt6";
    static const char *const kCommonPluginInterface = "org.deepin.plugin.common.qt6";
    static const char *const kPluginCore = "ddplugin-core";
    static const char *const kLibCore = "libdd-core-plugin.so";
#endif


static constexpr int kMemoryThreshold { 80 * 1024 };   // 80MB
static constexpr int kTimerInterval { 60 * 1000 };   // 1 min

DFMBASE_USE_NAMESPACE

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppDesktop) << "Load plugins but dconfig failed: " << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppDesktop) << QString("Load plugins path : %1").arg(pluginsDir);
    qApp->setProperty("DFM_BUILD_PLUGIN_DIR", pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/desktop");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_DESKTOP_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_DESKTOP_EDGE_DIR);
#endif

    qCInfo(logAppDesktop) << "Using plugins dir:" << pluginsDirs;

    // TODO(xust): the GVolumeMonitor object MUST be initialized in MAIN thread, so a initialize operation is added in dbusregister::initialize.
    // the function `DFMIO::DFMUtils::fileIsRemovable` indirectly initialized the GVolumeMonitor object and the function is invoked everywhere.
    // solve the indirectly initialize issue and then push the plugin to lazy list.
    static const QStringList kLazyLoadPluginNames { "ddplugin-wallpapersetting", "dfmplugin-bookmark", "dfmplugin-propertydialog",
                                                    "dfmplugin-tag", "dfmplugin-burn", "dfmplugin-dirshare", "dfmplugin-emblem",
                                                    "dfmplugin-filepreview" };

    QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "desktop.blackList").toStringList() };
#ifdef COMPILE_ON_V23
    if (DFMBASE_NAMESPACE::WindowUtils::isWayLand()) {
        qCInfo(logAppDesktop) << "disable background by TreeLand";
        if (!blackNames.contains("ddplugin-background")) {
            blackNames.append("ddplugin-background");
        }
    }
#endif
    DPF_NAMESPACE::LifeCycle::initialize({ kDesktopPluginInterface, kCommonPluginInterface }, pluginsDirs, blackNames, kLazyLoadPluginNames);

    qCInfo(logAppDesktop) << "Depend library paths:" << DApplication::libraryPaths();
    qCInfo(logAppDesktop) << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore)) {
        qCWarning(logAppDesktop) << corePlugin->fileName() << "is not" << kLibCore;
        return false;
    }
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
        return false;

    return true;
}

#ifdef COMPILE_ON_V23
#    define SESSION_MANAGER_SERVICE "org.deepin.dde.SessionManager1"
#    define SESSION_MANAGER_PATH "org/deepin/dde/SessionManager1"
#    define SESSION_MANAGER_INTERFACE "org.deepin.dde.SessionManager1"
#else
#    define SESSION_MANAGER_SERVICE "com.deepin.SessionManager"
#    define SESSION_MANAGER_PATH "/com/deepin/SessionManager"
#    define SESSION_MANAGER_INTERFACE "com.deepin.SessionManager"
#endif

static void registerDDESession()
{
    const char *envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    QByteArray cookie = qgetenv(envName);
    qunsetenv(envName);

    if (!cookie.isEmpty()) {
        QDBusInterface iface(SESSION_MANAGER_SERVICE,
                             SESSION_MANAGER_PATH,
                             SESSION_MANAGER_INTERFACE,
                             QDBusConnection::sessionBus());
        iface.call("Register", QString(cookie));
    }
}

static void initLog()
{
#ifdef DTKCORE_CLASS_DConfigFile
    LoggerRules::instance().initLoggerRules();
#endif
    dpfLogManager->applySuggestedLogSettings();
}

static void checkUpgrade(DApplication *app)
{
    if (!dfm_upgrade::isNeedUpgrade())
        return;

    qCInfo(logAppDesktop) << "try to upgrade in desktop";
    QMap<QString, QString> args;
    args.insert("version", app->applicationVersion());
    args.insert(dfm_upgrade::kArgDesktop, "dde-desktop");

    QString lib;
    GetUpgradeLibraryPath(lib);

    int ret = dfm_upgrade::tryUpgrade(lib, args);
    if (ret < 0) {
        qCWarning(logAppDesktop) << "something error, exit current process." << app->applicationPid();
        _Exit(-1);
    } else if (ret == 0) {
        auto arguments = app->arguments();
        // remove first
        if (!arguments.isEmpty())
            arguments.pop_front();

        QDBusConnection::sessionBus().unregisterService(kDesktopServiceName);
        qCInfo(logAppDesktop) << "restart self " << app->applicationFilePath() << arguments;
        QProcess::startDetached(app->applicationFilePath(), arguments);
        _Exit(-1);
    }

    return;
}

static bool isDesktopEnable()
{
    bool enable = !(dfmbase::DConfigManager::instance()->value(
                                                               dfmbase::kDefaultCfgPath,
                                                               "dd.disabled",
                                                               false)
                            .toBool());
    return enable;
}

static void autoReleaseMemory()
{
    bool autoRelease = dfmbase::DConfigManager::instance()->value(dfmbase::kDefaultCfgPath, "dfm.memory.autorelease", true).toBool();
    if (!autoRelease)
        return;

    static QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [] {
        float memUsage = dfmbase::SysInfoUtils::getMemoryUsage(getpid());
        if (memUsage > kMemoryThreshold)
            malloc_trim(0);
    });

    timer.start(kTimerInterval);
}

static void waitingForKwin()
{
    qCWarning(logAppDesktop) << "start waiting kwin ";
    QElapsedTimer timer;
    timer.start();
    int maxTime = 2000;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    while (maxTime > 0) {
        if (sessionBus.interface()->isServiceRegistered("org.kde.KWin"))
            break;
        QThread::msleep(50);
        maxTime -= 50;
    }
    qint64 elapsed = timer.nsecsElapsed() / 1000000;
    qCWarning(logAppDesktop) << "waiting for kwin ready cost" << elapsed << "ms";
}

bool first_check_wayland_env()
{
#ifdef COMPILE_ON_V23
    if (qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == "TreeLand")
        return false;
#endif

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else {
        return false;
    }
}
int main(int argc, char *argv[])
{
    if (first_check_wayland_env()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        setenv("PULSE_PROP_media.role", "video", 1);
#ifndef __x86_64__
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#endif
    }

    initLog();
    QString mainTime = QDateTime::currentDateTime().toString();
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setApplicationVersion(BUILD_VERSION);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setProductIcon(QIcon::fromTheme("deepin-toggle-desktop"));
    a.setWindowIcon(QIcon::fromTheme("deepin-toggle-desktop"));
    {
        // load translation
        QString appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
        a.setApplicationDisplayName(a.translate("DesktopMain", "Desktop"));
        a.setQuitOnLastWindowClosed(false);
    }

    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    autoReleaseMemory();

    qCInfo(logAppDesktop) << "start desktop " << a.applicationVersion() << "pid" << getpid() << "parent id" << getppid()
                          << "argments" << a.arguments() << mainTime;

    {
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService(kDesktopServiceName)) {
            qCCritical(logAppDesktop) << "registerService Failed, maybe service exist" << conn.lastError();
            exit(0x0002);
        }

        DesktopDBusInterface *interface = new DesktopDBusInterface(&a);
        auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
        if (!conn.registerObject(kDesktopServicePath, kDesktopServiceInterface, interface, registerOptions)) {
            qCCritical(logAppDesktop) << "registerObject Failed" << conn.lastError();
            exit(0x0003);
        }
    }

    if (isDesktopEnable()) {
        checkUpgrade(&a);

        if (!pluginsLoad()) {
            qCCritical(logAppDesktop) << "Load pugin failed!";
            abort();
        }
    } else {
        qCWarning(logAppDesktop) << "desktop is disabled...";
    }

    QVariantMap startUpData {};
    startUpData.insert(DFMGLOBAL_NAMESPACE::DataPersistence::kDesktopLaunchTime, mainTime);
    Application::instance()->dataPersistence()->setValue(DFMGLOBAL_NAMESPACE::DataPersistence::kReportGroup,
                                                         DFMGLOBAL_NAMESPACE::DataPersistence::kDesktopStartUpReportKey,
                                                         startUpData);

    // Notify dde-desktop start up
    registerDDESession();

    // bug 236971 need to wait for kwin
    waitingForKwin();

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
