/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "daemonlibrary.h"

#include <QLibrary>
#include <QDebug>

DDP_GRANDSEARCHDAEMON_USE_NAMESPACE

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

    verFunc = (VerDaemon)lib->resolve("grandSearchDaemonAppVersion");
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion in" << libPath;
        delete lib;
        return false;
    }

    startFunc = (StartDaemon)lib->resolve("startGrandSearchDaemon");
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon in" << libPath;
        delete lib;
        return false;
    }

    stopFunc = (StopDaemon)lib->resolve("stopGrandSearchDaemon");
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
