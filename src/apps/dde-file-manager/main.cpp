/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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

#include "config.h" //cmake

#include "dfm-framework/lifecycle/lifecycle.h"
#include "dfm-framework/log/frameworklog.h"
#include "dfm-framework/log/logutils.h"
#include "dfm-framework/log/codetimecheck.h"
#include "dfm-framework/definitions/globaldefinitions.h"
#include "dfm-framework/event/eventcallproxy.h"

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

int pluginsLoad()
{
    dpfCheckTimeBegin();

    // set plugin iid from qt style
    dpf::LifeCycle::setPluginIID(PLUGIN_INTERFACE);

    // cmake out definitions "DFM_PLUGIN_PATH" and "DFM_BUILD_OUT_PLGUN_DIR"
    if (DApplication::applicationDirPath() == "/usr/bin") {
        // run dde-file-manager path is /usr/bin, use system install plugins
        qInfo() << "run application in /usr/bin, load system plugin";
        dpf::LifeCycle::setPluginPaths({DFM_PLUGIN_PATH});
    } else {
        // if debug and any read from cmake out build path
        qInfo() << "run application not /usr/bin, load debug plugin";
        dpf::LifeCycle::setPluginPaths({DFM_BUILD_OUT_PLGUN_DIR});
    }

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    dpf::LifeCycle::readPlugins();

    // 手动初始化Core插件
    auto corePlugin = dpf::LifeCycle::pluginMetaObj("core");
    if (!corePlugin.isNull() && corePlugin->fileName().contains("libcore.so")) {
        dpf::LifeCycle::loadPlugin(corePlugin);
    }
    // load plugins without core
    dpf::LifeCycle::loadPlugins();

    dpfCheckTimeEnd();

    return 0;
}

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);

    dpf::FrameworkLog::initialize();

    pluginsLoad();

    return a.exec();
}
