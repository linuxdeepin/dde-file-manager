/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "config.h"
#include "dfm-framework/log/frameworklog.h"
#include "dfm-framework/lifecycle/lifecycle.h"

#include <DApplication>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

/// @brief  默认插件 iid
#define SERVER_PLUGIN_INTERFACE "Deepin.Bundle.Server.org"

static bool pluginsLoad()
{
    dpfCheckTimeBegin();
    // set plugin iid from qt style
    dpf::LifeCycle::setPluginIID(SERVER_PLUGIN_INTERFACE);

    // cmake out definitions "DFM_PLUGIN_PATH" and "DFM_BUILD_OUT_PLGUN_DIR"
    if (DApplication::applicationDirPath() == "/usr/bin") {
        // run dde-file-manager path is /usr/bin, use system install plugins
        qInfo() << "Run application in /usr/bin, load system plugin";
        dpf::LifeCycle::setPluginPaths({DFM_PLUGIN_PATH});
    } else {
        // if debug and any read from cmake out build path
        qInfo() << "Run application not /usr/bin, load debug plugin";
        dpf::LifeCycle::setPluginPaths({DFM_BUILD_OUT_PLGUN_DIR});
    }

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    dpf::LifeCycle::readPlugins();

    // load server plugin

    dpfCheckTimeEnd();
    return true;
}

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName("Deepin");

    dpf::FrameworkLog::initialize();

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    return a.exec();
}
