// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   //cmake
#include "singleapplication.h"
#include "commandparser.h"

#include "tools/upgrade/builtininterface.h"

#include <dfm-base/dfm_plugin_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/loggerrules.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QIcon>
#include <QDir>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QTextCodec>
#endif
#include <QProcess>
#include <QTimer>

#include <signal.h>
#include <malloc.h>

#ifdef DFM_UI_TYPE_QML
#    include <dfm-gui/sharedqmlengine.h>
#    include <dfm-gui/windowmanager.h>
#    include <QQmlEngine>
#endif

Q_LOGGING_CATEGORY(logAppFileManager, "org.deepin.dde.filemanager.filemanager")

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

#define BUILD_VERSION ((QString(VERSION) == "") ? "6.0.0.0" : QString(VERSION))

// defualt plugin IID
static constexpr char kFmPluginInterface[] { "org.deepin.plugin.filemanager" };
static constexpr char kCommonPluginInterface[] { "org.deepin.plugin.common" };
static constexpr char kPluginCore[] { "dfmplugin-core" };
static constexpr char kLibCore[] { "libdfmplugin-core.so" };

static constexpr int kMemoryThreshold { 80 * 1024 };   // 80MB
static constexpr int kTimerInterval { 60 * 1000 };   // 1 min

/* Within an SSH session, I can use gvfs-mount provided that
 * dbus-daemon is launched first and the environment variable DBUS_SESSION_BUS_ADDRESS is set.
 *
 * gvfs-mount and other GVFS utilities must all talk to the same D-Bus session. Hence,
 * if you use multiple SSH sessions or otherwise use mounts across login sessions, you must:
 * - start D-Bus the first time it is needed, at the latest;
 * - take care not to let D-Bus end with the session, as long as there are mounted GVFS filesystems;
 * - reuse the existing D-Bus session at login time if there is one.
 * see https://unix.stackexchange.com/questions/44317/reuse-d-bus-sessions-across-login-sessions for that.
 */
static void setEnvForRoot()
{
    QProcess p;
    p.start("bash", QStringList() << "-c"
                                  << "echo $(dbus-launch --autolaunch $(cat /var/lib/dbus/machine-id))");
    p.waitForFinished();
    QString envName("DBUS_SESSION_BUS_ADDRESS");
    QString output(p.readAllStandardOutput());
    QStringList group(output.split(" "));
    for (const QString &vals : group) {
        const QStringList &envGroup = vals.split(",");
        for (const QString &env : envGroup) {
            int mid = env.indexOf("=");
            if (env.startsWith(envName) && mid >= envName.size()) {
                QString v(env.mid(mid + 1));
                QByteArray nameBytes(envName.toLocal8Bit());
                QByteArray vBytes(v.toLocal8Bit());
                int ret = setenv(nameBytes.data(), vBytes.data(), 1);
                qCDebug(logAppFileManager) << "set " << env << "=" << v << "ret=" << ret;
                return;
            }
        }
    }
}

static bool lazyLoadFilter(const QString &name)
{
    static const auto &kAllNames {
        DFMBASE_NAMESPACE::Plugins::Utils::filemanagerCorePlugins()
    };

    if (!kAllNames.contains(name))
        return true;
    return false;
}

static QStringList buildBlackNames()
{
    QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "filemanager.blackList").toStringList() };
    const QStringList &disableNames { DConfigManager::instance()->value(kPluginsDConfName, "filemanager.disablelist").toStringList() };
#ifndef ENABLE_SMB_IN_ADMIN
    /*
     * NOTE(xust): the secret manager cannot be launched in WAYLAND ADMIN mode,
     * which cause file-manager freeze when mount samba (dfm-mount using secret-manager
     * to save/get the password of samba by sync).
     * and the Admin mode is designed for operate files those normal user cannot write
     * and should be the smallest dfm, so remove the smb-browser plugin in Admin mode
     * */
    if (SysInfoUtils::isOpenAsAdmin() && !blackNames.contains("dfmplugin-smbbrowser"))
        blackNames << "dfmplugin-smbbrowser";
#endif

    std::copy_if(disableNames.begin(), disableNames.end(), std::back_inserter(blackNames), [blackNames](const QString &name) {
        return !blackNames.contains(name);
    });
    qCDebug(logAppFileManager) << "build all blacknames:" << blackNames;
    return blackNames;
}

#ifdef DFM_UI_TYPE_QML
static bool initQmlEngine()
{
    QSharedPointer<QQmlEngine> globalEngine = dfmgui::WindowManager::instance()->engine();
    if (!globalEngine) {
        return false;
    }

#    ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    globalEngine->addImportPath(pluginsDir + "/qml");
#    else
    globalEngine->addImportPath(DFM_QML_MODULE);
#    endif

    return true;
}
#endif

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppFileManager) << "Load plugins but dconfig failed: " << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppFileManager) << QString("Load plugins path : %1").arg(pluginsDir);
    pluginsDirs.push_back(pluginsDir + "/filemanager");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
#endif

    qCInfo(logAppFileManager) << "Using plugins dir:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::initialize({ kFmPluginInterface, kCommonPluginInterface }, pluginsDirs, buildBlackNames());

    // disbale lazy load if enbale headless
    bool enableHeadless { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.headless", false).toBool() };
    if (enableHeadless && CommandParser::instance().isSet("d"))
        qCDebug(logAppFileManager) << "hot launch";
    else
        DPF_NAMESPACE::LifeCycle::setLazyloadFilter(lazyLoadFilter);

    qCInfo(logAppFileManager) << "Depend library paths:" << DApplication::libraryPaths();
    qCInfo(logAppFileManager) << "Load plugin paths: " << DPF_NAMESPACE::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore))
        return false;
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin))
        return false;

    // start filemanager, must called it after core plugin loaded
    CommandParser::instance().bindEvents();

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins())
        return false;

    return true;
}

static void handleSIGTERM(int sig)
{
    qCWarning(logAppFileManager) << "break with !SIGTERM! " << sig;

    if (qApp) {
        // Don't use headless if SIGTERM, cause system shutdown blocked
        qApp->setProperty("SIGTERM", true);
        qApp->quit();
    }
}

static void handleSIGPIPE(int sig)
{
    qCCritical(logAppFileManager) << "ignore !SIGPIPE! " << sig;
}

static void initEnv()
{
    // for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    if (qEnvironmentVariable("CLUTTER_IM_MODULE") == QStringLiteral("fcitx")) {
        setenv("QT_IM_MODULE", "fcitx", 1);
    }

    if (SysInfoUtils::isOpenAsAdmin())
        setEnvForRoot();
}

static void initLog()
{
#ifdef DTKCORE_CLASS_DConfigFile
    LoggerRules::instance().initLoggerRules();
#endif
    dpfLogManager->applySuggestedLogSettings();
}

static void checkUpgrade(SingleApplication *app)
{
    if (!dfm_upgrade::isNeedUpgrade())
        return;

    qCInfo(logAppFileManager) << "try to upgrade in file manager";
    QMap<QString, QString> args;
    args.insert("version", app->applicationVersion());
    args.insert(dfm_upgrade::kArgFileManger, "dde-file-manager");

    QString lib;
    GetUpgradeLibraryPath(lib);

    int ret = dfm_upgrade::tryUpgrade(lib, args);
    if (ret < 0) {
        qCWarning(logAppFileManager) << "something error, exit current process." << app->applicationPid();
        _Exit(-1);
    } else if (ret == 0) {
        // restart self
        app->closeServer();

        auto odlArgs = app->arguments();
        // remove first
        if (!odlArgs.isEmpty())
            odlArgs.pop_front();
        qCInfo(logAppFileManager) << "restart self " << app->applicationFilePath() << odlArgs;
        QProcess::startDetached(app->applicationFilePath(), odlArgs);
        _Exit(-1);
    }

    return;
}

static void autoReleaseMemory()
{
    bool autoRelease = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.memory.autorelease", true).toBool();
    if (!autoRelease)
        return;

    static QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [] {
        float memUsage = SysInfoUtils::getMemoryUsage(getpid());
        if (memUsage > kMemoryThreshold)
            malloc_trim(0);
    });

    timer.start(kTimerInterval);
}

int main(int argc, char *argv[])
{
    initEnv();

    // Warning: set log filter must before QApplication inited
    initLog();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
#endif

    SingleApplication a(argc, argv);

    a.setOrganizationName(ORGANIZATION_NAME);
    a.loadTranslator();
    a.setApplicationDisplayName(a.translate("Application", "File Manager"));
    a.setApplicationVersion(BUILD_VERSION);
    a.setProductIcon(QIcon::fromTheme("dde-file-manager"));
    a.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    a.setApplicationDescription(a.translate("Application", "File Manager is a powerful and "
                                                           "easy-to-use file management tool, "
                                                           "featured with searching, copying, "
                                                           "trash, compression/decompression, file property "
                                                           "and other useful functions."));
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    autoReleaseMemory();

    CommandParser::instance().process();
    qCInfo(logAppFileManager) << "App version: " << BUILD_VERSION;
    // working dir
    if (CommandParser::instance().isSet("w")) {
        QDir::setCurrent(CommandParser::instance().value("w"));
    }

    // open as root
    if (CommandParser::instance().isSet("r")) {
        qCInfo(logAppFileManager) << "Open as admin";
        a.openAsAdmin();
        return 0;
    }

    if (CommandParser::instance().isSet("h") || CommandParser::instance().isSet("v")) {
        return a.exec();
    }

    const QString &uniqueKey = a.applicationName();

    bool isSingleInstance = true;

    if (!SysInfoUtils::isOpenAsAdmin())
        isSingleInstance = a.setSingleInstance(uniqueKey);

    if (isSingleInstance) {
        // check upgrade
        checkUpgrade(&a);

#ifdef DFM_UI_TYPE_QML
        if (!initQmlEngine()) {
            qCCritical(logAppFileManager) << "init QQmlEngine failed!";
            abort();
        }
#endif
        if (!pluginsLoad()) {
            qCCritical(logAppFileManager) << "Load pugin failed!";
            abort();
        }
        signal(SIGTERM, handleSIGTERM);
        signal(SIGPIPE, handleSIGPIPE);
    } else {
        qCInfo(logAppFileManager) << "new client";
        a.handleNewClient(uniqueKey);
        return 0;
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();

    bool enableHeadless { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.headless", false).toBool() };
    bool isSigterm { qApp->property("SIGTERM").toBool() };
    if (!isSigterm && enableHeadless && !SysInfoUtils::isOpenAsAdmin()) {
        a.closeServer();
        QProcess::startDetached(QString(argv[0]), { "-d" });
    }

    return ret;
}
