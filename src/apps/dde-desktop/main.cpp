// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopdbusinterface.h"

#include "config.h"   //cmake
#include "tools/upgrade/builtininterface.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/sysinfoutils.h>

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

#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <malloc.h>

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
static const char *const kDesktopPluginInterface = "org.deepin.plugin.desktop";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
static const char *const kPluginCore = "ddplugin-core";
static const char *const kLibCore = "libddplugin-core.so";

static constexpr int kMemoryThreshold { 80 * 1024 };   // 80MB
static constexpr int kTimerInterval { 60 * 1000 };   // 1 min

static bool pluginsLoad()
{
    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qInfo() << QString("Load plugins path : %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/desktop");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_DESKTOP_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_DESKTOP_EDGE_DIR);
#endif

    qInfo() << "Using plugins dir:" << pluginsDirs;

    // TODO(xust): the GVolumeMonitor object MUST be initialized in MAIN thread, so a initialize operation is added in dbusregister::initialize.
    // the function `DFMIO::DFMUtils::fileIsRemovable` indirectly initialized the GVolumeMonitor object and the function is invoked everywhere.
    // solve the indirectly initialize issue and then push the plugin to lazy list.
    static const QStringList kLazyLoadPluginNames { "ddplugin-wallpapersetting", "dfmplugin-bookmark", "dfmplugin-propertydialog",
                                                    "dfmplugin-tag", "dfmplugin-burn", "dfmplugin-dirshare", "dfmplugin-emblem",
                                                    "dfmplugin-filepreview" };

    DPF_NAMESPACE::LifeCycle::initialize({ kDesktopPluginInterface, kCommonPluginInterface }, pluginsDirs, {}, kLazyLoadPluginNames);

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
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    dpfLogManager->setLogFormat(logFormat);
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

static void checkUpgrade(DApplication *app)
{
    if (!dfm_upgrade::isNeedUpgrade())
        return;

    qInfo() << "try to upgrade in desktop";
    QMap<QString, QString> args;
    args.insert("version", app->applicationVersion());
    args.insert(dfm_upgrade::kArgDesktop, "dde-desktop");

    QString lib;
    GetUpgradeLibraryPath(lib);

    int ret = dfm_upgrade::tryUpgrade(lib, args);
    if (ret < 0) {
        qWarning() << "something error, exit current process." << app->applicationPid();
        _Exit(-1);
    } else if (ret == 0) {
        auto arguments = app->arguments();
        // remove first
        if (!arguments.isEmpty())
            arguments.pop_front();

        QDBusConnection::sessionBus().unregisterService(kDesktopServiceName);
        qInfo() << "restart self " << app->applicationFilePath() << arguments;
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

int main(int argc, char *argv[])
{
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
    initLog();
    autoReleaseMemory();

    qInfo() << "start desktop " << a.applicationVersion() << "pid" << getpid() << "parent id" << getppid()
            << "argments" << a.arguments() << mainTime;

    {
        QDBusConnection conn = QDBusConnection::sessionBus();

        if (!conn.registerService(kDesktopServiceName)) {
            qCritical() << "registerService Failed, maybe service exist" << conn.lastError();
            exit(0x0002);
        }

        DesktopDBusInterface *interface = new DesktopDBusInterface(&a);
        auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
        if (!conn.registerObject(kDesktopServicePath, kDesktopServiceInterface, interface, registerOptions)) {
            qCritical() << "registerObject Failed" << conn.lastError();
            exit(0x0003);
        }

        // Notify dde-desktop start up
        registerDDESession();
    }

    if (isDesktopEnable()) {
        checkUpgrade(&a);

        if (!pluginsLoad()) {
            qCritical() << "Load pugin failed!";
            abort();
        }
    } else {
        qWarning() << "desktop is disabled...";
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
