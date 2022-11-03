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
#include "daemonplugin.h"
#include "daemonlibrary.h"

#include <QCoreApplication>

#include <QDir>
#include <QTimer>

DDP_GRANDSEARCHDAEMON_USE_NAMESPACE

void DaemonPlugin::initialize()
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
        return;
    }

    library = lib;
    lib->moveToThread(qApp->thread());
}

bool DaemonPlugin::start()
{
    if (library) {
        auto lis = dpfListener;
        connect(lis, &dpf::Listener::pluginsStarted, library, [this](){
            qInfo() << "ready to start grand seach after 3 seconds.";
            QTimer::singleShot(3000, library, [this](){
                qInfo() << "start grand search daemon" << library->version();
                if (library->start(0, nullptr) != 0) {
                    qWarning() << "fail to start.";
                    library->deleteLater();
                    library = nullptr;
                }
            });
        });
    }
    return true;
}

void DaemonPlugin::stop()
{
    if (library) {
        library->stop();
        delete library;
    }

    library = nullptr;
}
