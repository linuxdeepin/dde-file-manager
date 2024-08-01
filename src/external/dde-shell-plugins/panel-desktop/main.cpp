// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: CC0-1.0
#include "desktopdbusinterface.h"

#include "config.h"   //cmake
// TODO: #Qt6 upgrade

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <dde-shell/panel.h>
#include <dde-shell/pluginfactory.h>

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DPathBuf>
#include <DStandardPaths>

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

#include <iostream>
#include <algorithm>
#include <unistd.h>

Q_LOGGING_CATEGORY(logAppDesktop, "org.deepin.dde.filemanager.desktop")

DS_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dde_desktop;

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const kDesktopPluginInterface = "org.deepin.plugin.desktop.qt6";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common.qt6";
static const char *const kPluginCore = "ddplugin-core";
static const char *const kLibCore = "libdd-core-plugin.so";

#define SESSION_MANAGER_SERVICE "org.deepin.dde.SessionManager1"
#define SESSION_MANAGER_PATH "org/deepin/dde/SessionManager1"
#define SESSION_MANAGER_INTERFACE "org.deepin.dde.SessionManager1"

namespace desktop {

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
    if (qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == "TreeLand") {
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

static bool isDesktopEnable()
{
    bool enable = !(dfmbase::DConfigManager::instance()->value(
                                                               dfmbase::kDefaultCfgPath,
                                                               "dd.disabled",
                                                               false)
                            .toBool());
    return enable;
}

// we should wait for dtkgui to have a "proper" loadTranslation() to use.
static QStringList translationDir()
{
    QList<QString> translateDirs;
    QString appName { "dde-file-manager" };
    //("/home/user/.local/share", "/usr/local/share", "/usr/share")
    const QStringList dataDirs(DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    for (const auto &path : dataDirs) {
        DPathBuf pathBuf(path);
        translateDirs << (pathBuf / appName / "translations").toString();
    }
    return translateDirs;
}

static bool main()
{
    QString mainTime = QDateTime::currentDateTime().toString();
    qCInfo(logAppDesktop) << "start desktop " << qApp->applicationVersion() << "pid" << ::getpid() << "parent id" << getppid()
                          << "argments" << qApp->arguments() << mainTime;

    if (isDesktopEnable()) {
        if (!pluginsLoad()) {
            qCCritical(logAppDesktop) << "Load pugin failed!";
            return false;
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

    return true;
}

}   // namespace desktop

class DesktopPanel : public DPanel
{
    Q_OBJECT
public:
    explicit DesktopPanel(QObject *parent)
        : DPanel(parent)
    {
    }

    ~DesktopPanel()
    {
        DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    }

    bool load() override
    {
        // Translator
        DGuiApplicationHelper::loadTranslator(QStringLiteral("dde-file-manager"),
                                              desktop::translationDir(),
                                              { QLocale::system() });

        // DBus
        {
            QDBusConnection conn = QDBusConnection::sessionBus();

            if (!conn.registerService(kDesktopServiceName)) {
                qCWarning(logAppDesktop) << "registerService Failed, maybe service exist" << conn.lastError();
                return false;
            }

            DesktopDBusInterface *interface = new DesktopDBusInterface(qApp);
            auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
            if (!conn.registerObject(kDesktopServicePath, kDesktopServiceInterface, interface, registerOptions)) {
                qCWarning(logAppDesktop) << "registerObject Failed" << conn.lastError();
                return false;
            }
        }
        return DPanel::load();
    }

    bool init() override
    {
        DPanel::init();
        return desktop::main();
    }
};

D_APPLET_CLASS(DesktopPanel)

#include "main.moc"
