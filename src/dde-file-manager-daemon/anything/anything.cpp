// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anything/anything.h"
#include <QLibrary>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QCoreApplication>

static bool startAnything()
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
    AnythingObj ins = (AnythingObj) backendLib.resolve("fireAnything");
    if (ins) {
        ins();
        qInfo() << "found export func 'fireAnything' and load anything backend OK!!";
    } else {
        qInfo() << "Did not find export func 'fireAnything', please check deepin-anything-server lib version(>=6.0.1)";
    }

    return true;
}

static void stopAnything()
{
    // unload kernel module vfs_monitor
    // the logic inside anything causes rmmod to exit after the daemon has exited
    // so this does not use QProcess::start()
    QProcess::startDetached("rmmod", { "vfs_monitor" });
    qInfo() << "initiate unloading of kernel modules vfs_monitor.";
}

void initAnything(void)
{
    startAnything();

    // call stopAnything() when process exit
    qAddPostRoutine(stopAnything);
}
