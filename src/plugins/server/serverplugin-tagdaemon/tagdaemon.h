// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDAEMON_H
#define TAGDAEMON_H

#include "serverplugin_tagdaemon_global.h"

#include <dfm-framework/dpf.h>

class TagManagerDBus;

SERVERTAGDAEMON_BEGIN_NAMESPACE

class TagDBusWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void launchService();

private:
    QScopedPointer<TagManagerDBus> tagManager;
};

class TagDaemon : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "tagdaemon.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requestLaunch();

private:
    QThread workerThread;
};

SERVERTAGDAEMON_END_NAMESPACE

#endif   // TAGDAEMON_H
