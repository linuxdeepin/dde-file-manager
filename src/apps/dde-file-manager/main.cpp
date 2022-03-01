/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "utils/windowutils.h"

#include "services/filemanager/command/commandservice.h"

#include <dfm-framework/framework.h>

#include <DApplication>
#include <DMainWindow>

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QtGlobal>

#include <iostream>
#include <algorithm>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const kFmPluginInterface = "org.deepin.plugin.filemanager";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
static const char *const kPluginCore = "dfmplugin-core";
static const char *const kLibCore = "libdfmplugin-core.so";

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
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
    auto corePlugin = lifeCycle.pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore))
        return false;
    if (!lifeCycle.loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!lifeCycle.loadPlugins())
        return false;

    dpfCheckTimeEnd();

    return true;
}

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);
    a.setProductIcon(QIcon::fromTheme("dde-file-manager"));
    a.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/" + qApp->applicationName());
    a.setApplicationDescription(a.translate("Application", "File Manager is a powerful and "
                                                           "easy-to-use file management tool, "
                                                           "featured with searching, copying, "
                                                           "trash, compression/decompression, file property "
                                                           "and other useful functions."));
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    dpfInstance.initialize();

    commandServIns->process();

    // working dir
    if (commandServIns->isSet("w")) {
        QDir::setCurrent(commandServIns->value("w"));
    }

    // open as root
    if (commandServIns->isSet("r")) {
        if (WindowUtils ::isWayLand()) {
            QString cmd = "xhost";
            QStringList args;
            args << "+";
            QProcess p;
            p.start(cmd, args);
            p.waitForFinished();
        }

        QStringList args = a.arguments().mid(1);
        args.removeAll(QStringLiteral("-r"));
        args.removeAll(QStringLiteral("--root"));
        args.removeAll(QStringLiteral("-w"));
        args.removeAll(QStringLiteral("--working-dir"));
        QProcess::startDetached("dde-file-manager-pkexec", args, QDir::currentPath());
        return 0;
    }

    if (commandServIns->isSet("h") || commandServIns->isSet("v")) {
        return a.exec();
    }

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    return a.exec();
}
