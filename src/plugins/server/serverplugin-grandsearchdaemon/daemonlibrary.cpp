// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemonlibrary.h"

#include <QLibrary>
#include <QDebug>

SERVERPGRANDSEARCH_NAMESPACE_USE_NAMESPACE

DaemonLibrary::DaemonLibrary(const QString &filePtah, QObject *parent)
    : QObject(parent)
    , libPath(filePtah)
{

}

bool DaemonLibrary::load()
{
    if (library)
        return true;
    QLibrary *lib = new QLibrary(libPath, this);
    if (!lib->load()) {
        qCritical() << "can not load" << libPath << lib->errorString();
        delete lib;
        return false;
    }

    verFunc = reinterpret_cast<VerDaemon>(lib->resolve("grandSearchDaemonAppVersion"));
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion in" << libPath;
        delete lib;
        return false;
    }

    startFunc = reinterpret_cast<StartDaemon>(lib->resolve("startGrandSearchDaemon"));
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon in" << libPath;
        delete lib;
        return false;
    }

    stopFunc = reinterpret_cast<StopDaemon>(lib->resolve("stopGrandSearchDaemon"));
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon in" << libPath;
        delete lib;
        return false;
    }

    library = lib;
    return true;
}

void DaemonLibrary::unload()
{
    if (library) {
        library->unload();
        delete library;
        library = nullptr;
        startFunc = nullptr;
        stopFunc = nullptr;
    }
}

int DaemonLibrary::start(int argc, char *argv[])
{
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon";
        return -1;
    }
    return startFunc(argc, argv);
}

int DaemonLibrary::stop()
{
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon";
        return -1;
    }
    return stopFunc();
}

QString DaemonLibrary::version()
{
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion";
        return "";
    }
    return QString(verFunc());
}
