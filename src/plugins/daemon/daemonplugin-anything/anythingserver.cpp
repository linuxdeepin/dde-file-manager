// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingserver.h"
#include <QProcess>

DAEMONPANYTHING_USE_NAMESPACE

void AnythingPlugin::initialize()
{
    // define the deepin anything backend share library.
    backendLib = new QLibrary("deepin-anything-server-lib");
}

bool AnythingPlugin::start()
{
    // load share library and check status.
    backendLib->load();
    if (!backendLib->isLoaded()) {
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
    AnythingObj ins = (AnythingObj)backendLib->resolve("fireAnything");
    if (ins) {
        ins();
        qInfo() << "found export func 'fireAnything' and load anything backend OK!!";
    } else {
        qInfo() << "Did not find export func 'fireAnything', please check deepin-anything-server lib version(>=6.0.1)";
    }

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

    // define the anything backend instance fucntion.
    typedef void (*AnythingObj)();

    //resolve the anything backend instance down function.
    AnythingObj down = (AnythingObj)backendLib->resolve("downAnything");
    if (down) {
        down();
        qInfo() << "found export func 'downAnything'";
    }

    // unload this share library,
    if (backendLib->isLoaded()) {
        backendLib->unload();
        qInfo() << "unloaded deepin-anything-server-lib";
    }
    delete backendLib;
}
