// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGER1_H
#define FILEMANAGER1_H

#include "serverplugin_filemanager1_global.h"

#include <dfm-framework/dpf.h>

class FileManager1DBus;

SERVERFILEMANAGER1_BEGIN_NAMESPACE

class FileManager1DBusWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void launchService();

private:
    QScopedPointer<FileManager1DBus> filemanager1;
};

class FileManager1 : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "filemanager1.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requestLaunch();

private:
    QThread workerThread;
};

SERVERFILEMANAGER1_END_NAMESPACE

#endif   // FILEMANAGER1_H
