// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"   //cmake

#include <DApplication>
#include <QDir>
#include <QIcon>
#include <QTimer>

#include <dfm-base/dfm_plugin_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/loggerrules.h>

#include <dfm-framework/dpf.h>

#include <signal.h>

Q_LOGGING_CATEGORY(logAppDialogX11, "org.deepin.dde.filemanager.filedialog-x11")

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

static constexpr char kDialogPluginInterface[] { "org.deepin.plugin.filedialog" };
static constexpr char kFmPluginInterface[] { "org.deepin.plugin.filemanager" };
static constexpr char kCommonPluginInterface[] { "org.deepin.plugin.common" };

static constexpr char kDialogCorePluginName[] { "filedialog-core-plugin" };
static constexpr char kDialogCoreLibName[] { "libfiledialog-core-plugin.so" };
static constexpr char kDFMCorePluginName[] { "dfmplugin-core" };
static constexpr char kDFMCoreLibName[] { "libdfm-core-plugin.so" };

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

static void initEnv()
{
    // for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    if (qEnvironmentVariable("CLUTTER_IM_MODULE") == QStringLiteral("fcitx")) {
        setenv("QT_IM_MODULE", "fcitx", 1);
    }
}

static bool singlePluginLoad(const QString &pluginName, const QString &libName)
{
    auto plugin = DPF_NAMESPACE::LifeCycle::pluginMetaObj(pluginName);
    if (plugin.isNull())
        return false;
    if (!plugin->fileName().contains(libName))
        return false;
    if (!DPF_NAMESPACE::LifeCycle::loadPlugin(plugin))
        return false;

    return true;
}

static bool lazyLoadFilter(const QString &name)
{
    static const auto &kAllNames {
        DFMBASE_NAMESPACE::Plugins::Utils::filemanagerCorePlugins()
    };

    if (!kAllNames.contains(name) && (name != kDialogCorePluginName))
        return true;
    return false;
}

static bool blackListFilter(const QString &name)
{
    static const auto &kBlackNames { DConfigManager::instance()->value(kPluginsDConfName, "filedialog.blackList").toStringList() };
    if (kBlackNames.contains(name))
        return true;
    static const auto &kAllNames {
        DFMBASE_NAMESPACE::Plugins::Utils::filemanagerAllPlugins()
    };

    {
        // FIXME(zhangsheng) FIXME(xust): find another way to solve this.
        static const QStringList tmpWhiteList {
            "dfmplugin-disk-encrypt"
        };
        if (tmpWhiteList.contains(name))
            return false;
    }

    if (!kAllNames.contains(name) && (name != kDialogCorePluginName))
        return true;
    return false;
}

static bool pluginsLoad()
{
    QString msg;
    if (!DConfigManager::instance()->addConfig(kPluginsDConfName, &msg))
        qCWarning(logAppDialogX11) << "pluginsLoad: Failed to load plugins dconfig:" << msg;

    QStringList pluginsDirs;
#ifdef QT_DEBUG
    const QString &pluginsDir { DFM_BUILD_PLUGIN_DIR };
    qCInfo(logAppDialogX11) << "pluginsLoad: Using debug plugins path:" << pluginsDir;
    pluginsDirs.push_back(pluginsDir + "/filemanager");
    pluginsDirs.push_back(pluginsDir + "/common");
    pluginsDirs.push_back(pluginsDir + "/filedialog");
#else
    pluginsDirs << QString(DFM_PLUGIN_COMMON_CORE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_CORE_DIR)
                << QString(DFM_PLUGIN_COMMON_EDGE_DIR)
                << QString(DFM_PLUGIN_FILEMANAGER_EDGE_DIR);
#endif

    qCInfo(logAppDialogX11) << "pluginsLoad: Using plugins directories:" << pluginsDirs;
    DPF_NAMESPACE::LifeCycle::initialize({ kDialogPluginInterface,
                                           kFmPluginInterface,
                                           kCommonPluginInterface },
                                         pluginsDirs);
    DPF_NAMESPACE::LifeCycle::setLazyloadFilter(lazyLoadFilter);
    DPF_NAMESPACE::LifeCycle::setBlackListFilter(blackListFilter);
    DPF_NAMESPACE::LifeCycle::registerQtVersionInsensitivePlugins(Plugins::Utils::filemanagerAllPlugins());

    qCInfo(logAppDialogX11) << "pluginsLoad: Library paths:" << DApplication::libraryPaths();
    qCInfo(logAppDialogX11) << "pluginsLoad: Plugin paths:" << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!DPF_NAMESPACE::LifeCycle::readPlugins()) {
        qCCritical(logAppDialogX11) << "pluginsLoad: Failed to read plugins";
        return false;
    }

    // We should make sure that the core plugin is loaded first
    if (!singlePluginLoad(kDialogCorePluginName, kDialogCoreLibName)) {
        qCCritical(logAppDialogX11) << "pluginsLoad: Failed to load dialog core plugin:" << kDialogCorePluginName;
        return false;
    }
    if (!singlePluginLoad(kDFMCorePluginName, kDFMCoreLibName)) {
        qCCritical(logAppDialogX11) << "pluginsLoad: Failed to load DFM core plugin:" << kDFMCorePluginName;
        return false;
    }

    // load plugins without core
    if (!DPF_NAMESPACE::LifeCycle::loadPlugins()) {
        qCCritical(logAppDialogX11) << "pluginsLoad: Failed to load remaining plugins";
        return false;
    }

    qCInfo(logAppDialogX11) << "pluginsLoad: All plugins loaded successfully";
    return true;
}

static void handleSIGTERM(int sig)
{
    // 这里处理时不能有任何的内存分配，可能会出现卡死，或者崩溃
    if (qApp) {
        qApp->quit();
    }
}

int main(int argc, char *argv[])
{
    initEnv();
    initLogFilter();

    DApplication a(argc, argv);

    // BUG-278055
    initLogSetting();

    a.setLayoutDirection(Qt::LeftToRight);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setQuitOnLastWindowClosed(false);
    a.setProperty("GTK", true);   // see: FileDialogHandle::winId()
    a.setWindowIcon(QIcon::fromTheme("dde-file-manager"));

    {
        // load translation
        auto appName = a.applicationName();
        a.setApplicationName("dde-file-manager");
        a.loadTranslator();
        a.setApplicationName(appName);
    }

    qCInfo(logAppDialogX11) << "main: X11 file dialog application started, version:" << a.applicationVersion();

    signal(SIGTERM, handleSIGTERM);

    DPF_NAMESPACE::backtrace::installStackTraceHandler();

    if (!pluginsLoad()) {
        qCCritical(logAppDialogX11) << "main: Failed to load plugins, terminating application";
        Q_ASSERT_X(false, "pluginsLoad", "Failed to load plugins");
    }

    qCInfo(logAppDialogX11) << "main: Application initialization completed successfully";
    int ret { a.exec() };

    qCInfo(logAppDialogX11) << "main: Shutting down plugins";
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();

    qCInfo(logAppDialogX11) << "main: Application exiting normally with code:" << ret;
    return ret;
}
