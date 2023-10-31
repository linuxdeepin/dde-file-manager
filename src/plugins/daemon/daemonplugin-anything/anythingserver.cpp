// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingserver.h"
#include <QStandardPaths>

DAEMONPANYTHING_USE_NAMESPACE

static bool loadKernelModule()
{
    QProcess process;
    bool ret = false;

    process.start("modprobe", { "vfs_monitor" }, QIODevice::ReadOnly);
    if (process.waitForFinished(1000)) {
        ret = process.exitCode() == 0;
        qInfo() << "load kernel module vfs_monitor" << (ret ? "succeeded." : "failed.") << "exitcode(" << process.exitCode() << ")";
    } else {
        qInfo() << "load kernel module vfs_monitor timed out.";
    }

    return ret;
}

static bool unloadKernelModule()
{
    QProcess process;
    bool ret = false;

    process.start("rmmod", { "vfs_monitor" }, QIODevice::ReadOnly);
    if (process.waitForFinished(1000)) {
        ret = process.exitCode() == 0;
        qInfo() << "unload kernel module vfs_monitor" << (ret ? "succeeded." : "failed.") << "exitcode(" << process.exitCode() << ")";
    } else {
        qInfo() << "unload kernel module vfs_monitor timed out.";
    }

    return ret;
}

static bool startAnythingByProcess(QProcess **server)
{
    if (QStandardPaths::findExecutable("deepin-anything-server").isEmpty()) {
        qInfo() << "deepin-anything-server do not exist, maybe the deepin-anything-server has not been installed.";
        return false;
    }

    if (!loadKernelModule())
        return false;

    *server = new QProcess();
    (*server)->start("deepin-anything-server", QStringList(), QIODevice::NotOpen);
    if (!(*server)->waitForStarted(3*1000)) {
        qInfo() << "start deepin-anything-server fail.";
        unloadKernelModule();
        delete *server;
        *server = nullptr;
        return false;
    }

    qInfo() << "started deepin-anything-server.";
    return true;
}

void AnythingMonitorThread::run()
{
    unsigned long restart_cycle = 10;

    qInfo() << "started deepin-anything-server monitor thread.";
    while (true) {
        if (!server->waitForFinished(-1)) {
            qInfo() << "wait deepin-anything-server quit fail.";
            break;
        }
        qInfo() << "found deepin-anything-server quit.";
        delete server;
        server = nullptr;
        if (*stopped) {
            qInfo() << "found plugin stopped.";
            break;
        }
        qInfo() << "restart deepin-anything-server after" << restart_cycle << "seconds";
        QThread::sleep(restart_cycle);
        if (!startAnythingByProcess(&server))
            break;
    }
}

void AnythingPlugin::initialize()
{
    backendLib = nullptr;
    stopped = true;
}

bool AnythingPlugin::start()
{
    QProcess *server;
    bool ret;
    AnythingMonitorThread* thread;

    if (!stopped)
        return true;

    if (startAnythingByProcess(&server)) {
        thread = new AnythingMonitorThread(server, &stopped);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
        ret = true;
    }
    else {
        ret = startAnythingByLib();
    }

    stopped = !ret;
    return ret;
}

void AnythingPlugin::stop()
{
    if (stopped)
        return;

    stopped = true;
    unloadKernelModule();
    stopAnythingByLib();
}

bool AnythingPlugin::startAnythingByLib()
{
    // define the deepin anything backend share library.
    backendLib = new QLibrary("deepin-anything-server-lib");

    // load share library and check status.
    backendLib->load();
    if (!backendLib->isLoaded()) {
        qInfo() << "load deepin-anything-server-lib.so failed!!, maybe the deepin-anything-server has not been installed.";
        delete backendLib;
        backendLib = nullptr;
        return false;
    }

    if (!loadKernelModule()) {
        delete backendLib;
        backendLib = nullptr;
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

void AnythingPlugin::stopAnythingByLib()
{
    if (!backendLib)
        return;

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
    backendLib = nullptr;
}
