// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONLIBRARY_H
#define DAEMONLIBRARY_H

#include "serverplugin_grandsearchdaemon_global.h"

#include <QObject>

typedef int (*StartDaemon)(int argc, char *argv[]);
typedef int (*StopDaemon)();
typedef const char *(*VerDaemon)();
class QLibrary;

SERVERPGRANDSEARCH_NAMESPACE_BEGIN_NAMESPACE

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

SERVERPGRANDSEARCH_NAMESPACE_END_NAMESPACE

#endif // DAEMONLIBRARY_H
