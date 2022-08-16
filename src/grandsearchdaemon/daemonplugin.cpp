// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemonplugin.h"
#include "daemonlibrary.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

using namespace GrandSearch;

DaemonPlugin::DaemonPlugin(QObject *parent) : QObject(parent)
{

}

DaemonPlugin::~DaemonPlugin()
{
    stop();
}

bool DaemonPlugin::initialize()
{
    QString libPath;
    {
        auto defaultPath = GRANDSEARCHDAEMON_LIB_DIR;
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "DAEMON_LIB_BASE_DIR is not a string.");

        QDir dir(defaultPath);
        libPath = dir.absoluteFilePath("libdde-grand-search-daemon.so");
        qInfo() << "daemon lib path:" << libPath;
    }

    DaemonLibrary *lib = new DaemonLibrary(libPath);
    if (!lib->load()) {
        qWarning() << "fail to load grand search library.";
        delete lib;
        return false;
    }

    library = lib;
    return true;
}

bool DaemonPlugin::start()
{
    if (library) {
        qInfo() << "start grand search daemon" << library->version();
        if (library->start(0, nullptr) == 0) {
            return true;
        } else {
            qWarning() << "fail to start.";
            library->unload();
            delete library;
            library = nullptr;
        }
    }
    return false;
}

void DaemonPlugin::stop()
{
    if (library) {
        library->stop();
        library->unload();
        delete library;
    }

    library = nullptr;
}
