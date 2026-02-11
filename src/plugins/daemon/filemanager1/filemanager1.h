// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGER1_H
#define FILEMANAGER1_H

#include "daemonplugin_filemanager1_global.h"

#include <dfm-framework/dpf.h>

class FileManager1DBus;

DAEMONPFILEMANAGER1_BEGIN_NAMESPACE

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
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "filemanager1.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requestLaunch();

private:
    QThread workerThread;
};

DAEMONPFILEMANAGER1_END_NAMESPACE

#endif   // FILEMANAGER1_H
