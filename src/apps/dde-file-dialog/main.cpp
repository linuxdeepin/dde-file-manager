/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "config.h"   //cmake

#include <DApplication>
#include <QDir>
#include <QTextCodec>

#include <dfm-framework/framework.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

static constexpr char kDialogPluginInterface[] { "org.deepin.plugin.filedialog" };
static constexpr char kFmPluginInterface[] { "org.deepin.plugin.filemanager" };
static constexpr char kCommonPluginInterface[] { "org.deepin.plugin.common" };

static constexpr char kDialogCorePluginName[] { "filedialogplugin-core" };
static constexpr char kDialogCoreLibName[] { "libfiledialogplugin-core.so" };
static constexpr char kDFMCorePluginName[] { "dfmplugin-core" };
static constexpr char kDFMCoreLibName[] { "libdfmplugin-core.so" };

static void initLog()
{
    dpfInstance.log().registerConsoleAppender();
    dpfInstance.log().registerFileAppender();
}

static void initEnv()
{
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    if (qEnvironmentVariable("CLUTTER_IM_MODULE") == QStringLiteral("fcitx")) {
        setenv("QT_IM_MODULE", "fcitx", 1);
    }
}

static bool singlePluginLoad(const QString &pluginName, const QString &libName)
{
    auto &&lifeCycle = dpfInstance.lifeCycle();

    auto plugin = lifeCycle.pluginMetaObj(pluginName);
    if (plugin.isNull())
        return false;
    if (!plugin->fileName().contains(libName))
        return false;
    if (!lifeCycle.loadPlugin(plugin))
        return false;

    return true;
}

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    // TODO(zhangs): Configurable, maybe config file ?
    static const QStringList blackNameList {
        "dfmplugin-burn",
        "dfmplugin-dirshare",
        "dfmplugin-myshares",
        "dfmplugin-propertydialog",
        "dfmplugin-trashcore",
        "dfmplugin-trash",
        "dfmplugin-smbbrowser",
        "dfmplugin-filepreview",
        "dfmplugin-tag"
    };

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // don't load plugins of blackNameList
    lifeCycle.addBlackPluginNames(blackNameList);

    // set plugin iid from qt style
    lifeCycle.addPluginIID(kDialogPluginInterface);
    lifeCycle.addPluginIID(kFmPluginInterface);
    lifeCycle.addPluginIID(kCommonPluginInterface);

    QString pluginsDir(qApp->applicationDirPath() + "/../../plugins");
    if (!QDir(pluginsDir).exists()) {
        pluginsDir = DFM_PLUGIN_PATH;
    }
    qDebug() << "using plugins dir:" << pluginsDir;

    lifeCycle.setPluginPaths({ pluginsDir });

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!lifeCycle.readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    if (!singlePluginLoad(kDialogCorePluginName, kDialogCoreLibName)) {
        qWarning() << "Load" << kDialogCorePluginName << "failed";
        return false;
    }
    if (!singlePluginLoad(kDFMCorePluginName, kDFMCoreLibName)) {
        qWarning() << "Load" << kDFMCorePluginName << "failed";
        return false;
    }

    // load plugins without core
    if (!lifeCycle.loadPlugins())
        return false;

    dpfCheckTimeEnd();

    return true;
}

int main(int argc, char *argv[])
{
    initEnv();

    // Fixed the locale codec to utf-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setQuitOnLastWindowClosed(false);
    // TODO(zhangs): installTranslator

    dpfInstance.initialize();
    initLog();

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    return a.exec();
}
