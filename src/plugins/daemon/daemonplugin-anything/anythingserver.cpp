/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "anythingserver.h"
#include <QProcess>

DAEMONPANYTHING_USE_NAMESPACE

void AnythingPlugin::initialize()
{
}

bool AnythingPlugin::start()
{
    // define the deepin anything backend share library.
    QLibrary backendLib("deepin-anything-server-lib");

    // load share library and check status.
    backendLib.load();
    if (!backendLib.isLoaded()) {
        qInfo() << "load deepin-anything-server-lib.so failed!!, maybe the deepin-anything-server has not been installed.";
        return false;
    }

    // load kernel module vfs_monitor
    QProcess process;
    process.start("modprobe", { "vfs_monitor" }, QIODevice::ReadOnly);
    if (process.waitForFinished(1000)) {
        if (process.exitCode() == 0) {
            qInfo() << "load kernel module vfs_monitor succeeded.";
        } else {
            qInfo() << "load kernel module vfs_monitor failed.";
            return false;
        }
    } else {
        qInfo() << "load kernel module vfs_monitor timed out.";
        return false;
    }

    // define the anything backend instance fucntion.
    typedef void (*AnythingObj)();

    //resolve the anything backend instance fire function.
    AnythingObj ins = (AnythingObj)backendLib.resolve("fireAnything");
    if (ins) {
        ins();
        qInfo() << "found export func 'fireAnything' and load anything backend OK!!";
    } else {
        qInfo() << "Did not find export func 'fireAnything', please check deepin-anything-server lib version(>=6.0.1)";
    }

    // unload this share library, but donot unload at here.
    //backendLib.unload();
    return true;
}

void AnythingPlugin::stop()
{
    // unload kernel module vfs_monitor
    QProcess process;
    process.start("rmmod", { "vfs_monitor" }, QIODevice::ReadOnly);
    if (process.waitForFinished(1000)) {
        qInfo() << "unload kernel module vfs_monitor" << (process.exitCode() == 0 ? " succeeded." : " failed.");
    } else {
        qInfo() << "unload kernel module vfs_monitor timed out.";
    }
}
