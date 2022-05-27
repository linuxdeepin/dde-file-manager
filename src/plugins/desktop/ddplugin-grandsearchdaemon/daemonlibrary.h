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
#ifndef DAEMONLIBRARY_H
#define DAEMONLIBRARY_H

#include "ddplugin_grandsearchdaemon_global.h"

#include <QObject>

typedef int (*StartDaemon)(int argc, char *argv[]);
typedef int (*StopDaemon)();
typedef const char *(*VerDaemon)();
class QLibrary;

DDP_GRANDSEARCHDAEMON_BEGIN_NAMESPACE

class DaemonLibrary : public QObject
{
    Q_OBJECT
public:
    explicit DaemonLibrary(const QString &filePtah, QObject *parent = nullptr);
    bool load();
    void unload();
    int start(int argc, char *argv[]);
    int stop();
    QString version();
protected:
    StartDaemon startFunc = nullptr;
    StopDaemon stopFunc = nullptr;
    VerDaemon verFunc = nullptr;
    QString libPath;
    QLibrary *library = nullptr;
};

DDP_GRANDSEARCHDAEMON_END_NAMESPACE

#endif // DAEMONLIBRARY_H
