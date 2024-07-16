// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingserver.h"
#include <QStandardPaths>

namespace service_anything {
DFM_LOG_REISGER_CATEGORY(SERVICEANYTHING_NAMESPACE)

static bool loadKernelModule()
{
    QProcess process;
    bool ret = false;

    process.start("modprobe", { "vfs_monitor" }, QIODevice::ReadOnly);
    if (process.waitForFinished(1000)) {
        ret = process.exitCode() == 0;
        if (ret)
            fmInfo() << "load kernel module vfs_monitor succeeded.";
        else
            fmWarning() << "load kernel module vfs_monitor failed." << " exitcode(" << process.exitCode() << ")";
    } else {
        fmWarning() << "load kernel module vfs_monitor timed out.";
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
        if (ret)
            fmInfo() << "unload kernel module vfs_monitor succeeded.";
        else
            fmWarning() << "unload kernel module vfs_monitor failed." << " exitcode(" << process.exitCode() << ")";
    } else {
        fmWarning() << "unload kernel module vfs_monitor timed out.";
    }

    return ret;
}

static bool startAnythingByProcess(QProcess **server)
{
    if (QStandardPaths::findExecutable("deepin-anything-server").isEmpty()) {
        fmWarning() << "deepin-anything-server do not exist, maybe the deepin-anything-server has not been installed.";
        return false;
    }

    if (!loadKernelModule())
        return false;

    *server = new QProcess();
    (*server)->start("deepin-anything-server", QStringList(), QIODevice::NotOpen);
    if (!(*server)->waitForStarted(3 * 1000)) {
        fmWarning() << "start deepin-anything-server fail.";
        unloadKernelModule();
        delete *server;
        *server = nullptr;
        return false;
    }

    fmInfo() << "started deepin-anything-server.";
    return true;
}

void AnythingMonitorThread::run()
{
    unsigned long restart_cycle = 10;
    QProcess *server = nullptr;

    started = startAnythingByProcess(&server);
    sem.release();
    if (!started)
        return;

    fmInfo() << "start monitoring deepin-anything-server.";
    while (true) {
        if (!server->waitForFinished(-1)) {
            fmWarning() << "wait deepin-anything-server quit fail.";
            break;
        }
        fmInfo() << "found deepin-anything-server quit.";
        delete server;
        server = nullptr;
        if (*stopped) {
            fmInfo() << "found plugin stopped.";
            break;
        }
        fmInfo() << "restart deepin-anything-server after" << restart_cycle << "seconds";
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
    bool ret = true;
    AnythingMonitorThread *thread;

    if (!stopped)
        return true;

    thread = new AnythingMonitorThread(&stopped);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    if (!thread->waitStartResult())
        ret = startAnythingByLib();

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
        fmWarning() << "load deepin-anything-server-lib.so failed!!, maybe the deepin-anything-server has not been installed.";
        delete backendLib;
        backendLib = nullptr;
        return false;
    }

    fmWarning() << "load anything lib success, try load kernel module";
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
        fmInfo() << "found export func 'fireAnything' and load anything backend OK!!";
    } else {
        fmWarning() << "Did not find export func 'fireAnything', please check deepin-anything-server lib version(>=6.0.1)";
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
        fmInfo() << "found export func 'downAnything'";
    }

    // unload this share library,
    if (backendLib->isLoaded()) {
        backendLib->unload();
        fmInfo() << "unloaded deepin-anything-server-lib";
    }
    delete backendLib;
    backendLib = nullptr;
}

}   // namespace daemonplugin_anythin
