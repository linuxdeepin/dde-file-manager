// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   //cmake
#include "singleapplication.h"
#include "commandparser.h"
#include "dragmonitor.h"

#include "tools/upgrade/builtininterface.h"

#include <dfm-base/dfm_plugin_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/loggerrules.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QIcon>
#include <QDir>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QTextCodec>
#endif
#include <QProcess>
#include <QSocketNotifier>
#include <QTimer>

#include <signal.h>
#include <malloc.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(logAppFileManager, "org.deepin.dde.filemanager.filemanager")

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

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
static constexpr char kLibCore[] { "libdfm-core-plugin.so" };

static constexpr int kMemoryThreshold { 80 * 1024 };   // 80MB
static constexpr int kTimerInterval { 60 * 1000 };   // 1 min
// Use volatile sig_atomic_t as required by POSIX for variables modified in signal handlers
static volatile sig_atomic_t sigtermFlag { 0 };
// Self-pipe trick: fd[0]=read end (QSocketNotifier), fd[1]=write end (signal handler)
static int g_sigTermPipe[2] { -1, -1 };

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
    // 管理员模式可能丢失 QT_QPA_PLATFORM
    if (DFMBASE_NAMESPACE::WindowUtils::isX11() && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "dxcb");
    }

    QProcess p1;
    QProcess p2;

    // 首先执行 cat 命令获取 machine-id
    p1.start("cat", QStringList() << "/var/lib/dbus/machine-id");
    p1.waitForFinished();
    QString machineId = p1.readAllStandardOutput().trimmed();   // 去除多余的空白字符

    // 然后使用获取到的 machine-id 执行 dbus-launch
    p2.start("dbus-launch", QStringList() << "--autolaunch" << machineId);
    p2.waitForFinished();
    QString output = p2.readAllStandardOutput().trimmed();

    QStringList group(output.split('\n'));
    QString envName("DBUS_SESSION_BUS_ADDRESS");
    for (const QString &vals : group) {
        const QStringList &envGroup = vals.split(",");
        for (const QString &env : envGroup) {
            int mid = env.indexOf("=");
            if (env.startsWith(envName) && mid >= envName.size()) {
                QString v(env.mid(mid + 1));
                QByteArray nameBytes(envName.toLocal8Bit());
                QByteArray vBytes(v.toLocal8Bit());
                int ret = setenv(nameBytes.data(), vBytes.data(), 1);
                qCInfo(logAppFileManager) << "setEnvForRoot: Set D-Bus environment variable:" << env << "return code:" << ret;
                return;
            }
        }
    }
    qCWarning(logAppFileManager) << "setEnvForRoot: Failed to set D-Bus environment variable";
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

static bool lazyLoadFilterForHeadless(const QString &name)
{
    static const auto &kAllNames {
        DFMBASE_NAMESPACE::Plugins::Utils::filemanagerAllPlugins()
    };

    if (!kAllNames.contains(name))
        return true;
    return false;
}

static bool blackListFilter(const QString &name)
{
    static QStringList blackNames { DConfigManager::instance()->value(kPluginsDConfName, "filemanager.blackList").toStringList() };
    static bool isAdmin = SysInfoUtils::isOpenAsAdmin();
    static std::once_flag flag;
    std::call_once(flag, [] {
#ifndef ENABLE_SMB_IN_ADMIN
        /*
         * NOTE(xust): the secret manager cannot be launched in WAYLAND ADMIN mode,
         * which cause file-manager freeze when mount samba (dfm-mount using secret-manager
         * to save/get the password of samba by sync).
         * and the Admin mode is designed for operate files those normal user cannot write
         * and should be the smallest dfm, so remove the smb-browser plugin in Admin mode
         * */
        if (isAdmin && !blackNames.contains("dfmplugin-smbbrowser"))
            blackNames << "dfmplugin-smbbrowser";
#endif
        static QStringList disableNames { DConfigManager::instance()->value(kPluginsDConfName, "filemanager.disablelist").toStringList() };
        std::copy_if(disableNames.begin(), disableNames.end(), std::back_inserter(blackNames),
                     [](const QString &name) {
                         return !blackNames.contains(name);
                     });
        qCDebug(logAppFileManager) << "build all blacknames:" << blackNames;
    });

    if (blackNames.contains(name))
        return true;

    // Disable all third-party plugins when running the file manager as administrator
    static const QStringList &kAllNames { DFMBASE_NAMESPACE::Plugins::Utils::filemanagerAllPlugins() };
    return isAdmin && !kAllNames.contains(name);
}

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppFileManager) << "pluginsLoad: Failed to load plugins dconfig:" << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppFileManager) << "pluginsLoad: Using debug plugins path:" << pluginsDir;
    pluginsDirs.push_back(pluginsDir + "/filemanager");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir);
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
#endif

    qCInfo(logAppFileManager) << "pluginsLoad: Using plugins directories:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::initialize({ kFmPluginInterface, kCommonPluginInterface }, pluginsDirs);
    DPF_NAMESPACE::LifeCycle::setBlackListFilter(blackListFilter);
    DPF_NAMESPACE::LifeCycle::registerQtVersionInsensitivePlugins(Plugins::Utils::filemanagerAllPlugins());
    // disbale lazy load if enbale headless
    bool enableHeadless { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.headless", false).toBool() };
    if (enableHeadless && CommandParser::instance().isSet("d")) {
        qCInfo(logAppFileManager) << "pluginsLoad: Hot launch mode enabled, using headless filter";
        // Non-built-in plugins do not guarantee correctness (e.g. udrive)
        DPF_NAMESPACE::LifeCycle::setLazyloadFilter(lazyLoadFilterForHeadless);
    } else {
        qCDebug(logAppFileManager) << "pluginsLoad: Using standard lazy load filter";
        DPF_NAMESPACE::LifeCycle::setLazyloadFilter(lazyLoadFilter);
    }
    qCInfo(logAppFileManager) << "pluginsLoad: Library paths:" << DApplication::libraryPaths();
    qCInfo(logAppFileManager) << "pluginsLoad: Plugin paths:" << DPF_NAMESPACE::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins()) {
        qCCritical(logAppFileManager) << "pluginsLoad: Failed to read plugins";
        return false;
    }

    // We should make sure that the core plugin is loaded first
    auto corePlugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(kPluginCore);
    if (corePlugin.isNull()) {
        qCCritical(logAppFileManager) << "pluginsLoad: Core plugin not found:" << kPluginCore;
        return false;
    }
    if (!corePlugin->fileName().contains(kLibCore)) {
        qCCritical(logAppFileManager) << "pluginsLoad: Core plugin library mismatch, expected:" << kLibCore
                                      << "actual:" << corePlugin->fileName();
        return false;
    }
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(corePlugin)) {
        qCCritical(logAppFileManager) << "pluginsLoad: Failed to load core plugin:" << kPluginCore;
        return false;
    }

    qCInfo(logAppFileManager) << "pluginsLoad: Core plugin loaded successfully";

    // start filemanager, must called it after core plugin loaded
    CommandParser::instance().bindEvents();

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins()) {
        qCCritical(logAppFileManager) << "pluginsLoad: Failed to load remaining plugins";
        return false;
    }

    qCInfo(logAppFileManager) << "pluginsLoad: All plugins loaded successfully";
    return true;
}

static void handleSIGTERM(int /*sig*/)
{
    // Only async-signal-safe operations are allowed here.
    // write() is async-signal-safe; qApp->quit() is NOT, so we use self-pipe trick
    // to delegate the actual quit() call to the main event loop via QSocketNotifier.
    ::sigtermFlag = 1;
    const char byte = 1;
    // Ignore return value: if write fails we cannot do anything safe in a signal handler
    (void)::write(g_sigTermPipe[1], &byte, sizeof(byte));
}

static void handleSIGPIPE(int /*sig*/)
{
    // Intentionally empty: qCInfo is NOT async-signal-safe.
    // SIGPIPE is silently ignored; broken pipe errors are handled at the call site.
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

    if (SysInfoUtils::isOpenAsAdmin()) {
        qCInfo(logAppFileManager) << "initEnv: Running as admin, printing all environment variables:";
        QStringList envVars = QProcess::systemEnvironment();
        for (const QString &envVar : envVars) {
            qCInfo(logAppFileManager) << "ENV:" << envVar;
        }
        setEnvForRoot();
    }
}

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

static void checkUpgrade(SingleApplication *app)
{
    if (!dfm_upgrade::isNeedUpgrade())
        return;

    qCInfo(logAppFileManager) << "checkUpgrade: Starting upgrade process for file manager";
    QMap<QString, QString> args;
    args.insert("version", app->applicationVersion());
    args.insert(dfm_upgrade::kArgFileManger, "dde-file-manager");

    QString lib;
    GetUpgradeLibraryPath(lib);

    int ret = dfm_upgrade::tryUpgrade(lib, args);
    if (ret < 0) {
        qCCritical(logAppFileManager) << "checkUpgrade: Upgrade failed, exiting process, PID:" << app->applicationPid();
        _Exit(-1);
    } else if (ret == 0) {
        // restart self
        app->closeServer();

        auto odlArgs = app->arguments();
        // remove first
        if (!odlArgs.isEmpty())
            odlArgs.pop_front();
        qCInfo(logAppFileManager) << "checkUpgrade: Restarting application:" << app->applicationFilePath() << "with args:" << odlArgs;
        QProcess::startDetached(app->applicationFilePath(), odlArgs);
        _Exit(-1);
    }

    qCInfo(logAppFileManager) << "checkUpgrade: Upgrade process completed successfully";
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
    initLogFilter();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
#endif

    SingleApplication a(argc, argv);

    // BUG-278055
    initLogSetting();

    a.setLayoutDirection(Qt::LeftToRight);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.loadTranslator();
    a.setApplicationDisplayName(a.translate("Application", "File Manager"));
    a.setApplicationVersion(BUILD_VERSION);
    a.setProductIcon(QIcon::fromTheme("dde-file-manager"));
    a.setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    a.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    a.setApplicationDescription(a.translate("Application", "File Manager is a powerful and "
                                                           "easy-to-use file management tool, "
                                                           "featured with searching, copying, "
                                                           "trash, compression/decompression, file property "
                                                           "and other useful functions."));
    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    autoReleaseMemory();

    CommandParser::instance().process();
    qCInfo(logAppFileManager) << "main: File Manager started, version:" << BUILD_VERSION;
    // working dir
    if (CommandParser::instance().isSet("w")) {
        QString workingDir = CommandParser::instance().value("w");
        QDir::setCurrent(workingDir);
        qCInfo(logAppFileManager) << "main: Working directory set to:" << workingDir;
    }

    // open as root
    if (CommandParser::instance().isSet("r")) {
        qCInfo(logAppFileManager) << "main: Opening file manager as administrator";
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
        qCInfo(logAppFileManager) << "main: Running as primary instance";
        // check upgrade
        checkUpgrade(&a);

        if (!pluginsLoad()) {
            qCCritical(logAppFileManager) << "main: Failed to load plugins, terminating application";
            Q_ASSERT_X(false, "pluginsLoad", "Failed to load plugins");
        }
        // Set up self-pipe so the signal handler can safely wake the main event loop
        if (::pipe(g_sigTermPipe) != 0) {
            qCWarning(logAppFileManager) << "main: Failed to create SIGTERM self-pipe, falling back to direct quit()";
        } else {
            // QSocketNotifier runs in the main event loop — safe to call qApp->quit() here
            auto *sigTermNotifier = new QSocketNotifier(g_sigTermPipe[0], QSocketNotifier::Read, &a);
            QObject::connect(sigTermNotifier, &QSocketNotifier::activated, &a, [&a]() {
                char tmp;
                (void)::read(g_sigTermPipe[0], &tmp, sizeof(tmp));
                qCInfo(logAppFileManager) << "main: SIGTERM received via self-pipe, quitting main event loop";
                // Don't use headless if SIGTERM, cause system shutdown blocked
                a.quit();
            });
        }
        signal(SIGTERM, handleSIGTERM);
        signal(SIGPIPE, handleSIGPIPE);
    } else {
        qCInfo(logAppFileManager) << "main: Detected existing instance, forwarding to primary instance";
        a.handleNewClient(uniqueKey);
        return 0;
    }

    // NOTE: temp code!!!!!!!!!!!
    QScopedPointer<dfm_drag::DragMoniter> mo(new dfm_drag::DragMoniter);
    if (!SysInfoUtils::isOpenAsAdmin()) {
        qCDebug(logAppFileManager) << "main: Registering drag monitor D-Bus service";
        mo->registerDBus();
    }

    qCInfo(logAppFileManager) << "main: Application started successfully, PID:" << a.applicationPid();
    int ret { a.exec() };

    mo->unRegisterDBus();
    a.closeServer();
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();

    // Close self-pipe fds to release kernel resources
    if (g_sigTermPipe[0] != -1) {
        ::close(g_sigTermPipe[0]);
        ::close(g_sigTermPipe[1]);
        g_sigTermPipe[0] = g_sigTermPipe[1] = -1;
    }

    bool enableHeadless { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.headless", false).toBool() };
    if (!::sigtermFlag && enableHeadless && !SysInfoUtils::isOpenAsAdmin()) {
        qCInfo(logAppFileManager) << "main: Starting headless process for background operation";
        QProcess::startDetached(QString(argv[0]), { "-d" });
    }

    qCInfo(logAppFileManager) << "main: Application exiting with code:" << ret;
    return ret;
}
