// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchdaemon.h"
#include "daemonlibrary.h"

#include <QCoreApplication>

#include <QDir>
#include <QTimer>

SERVERPGRANDSEARCH_NAMESPACE_BEGIN_NAMESPACE

void GrandSearchDaemon::initialize()
{
    if (qApp->arguments().contains("--no-grandsearch")) {
        qInfo() << "no grand search...";
        return;
    }

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
        return;
    }

    library = lib;
    lib->moveToThread(qApp->thread());
}

bool GrandSearchDaemon::start()
{
    if (library) {
        qInfo() << "ready to start grand seach after 1 seconds.";
        QTimer::singleShot(1000, library, [this]() {
            qInfo() << "start grand search daemon" << library->version();
            if (library->start(0, nullptr) != 0) {
                qWarning() << "fail to start.";
                library->deleteLater();
                library = nullptr;
            }
        });
    }
    return true;
}

void GrandSearchDaemon::stop()
{
    if (library) {
        library->stop();
        delete library;
    }

    library = nullptr;
}

SERVERPGRANDSEARCH_NAMESPACE_END_NAMESPACE
