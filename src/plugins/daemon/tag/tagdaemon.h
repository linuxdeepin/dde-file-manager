// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDAEMON_H
#define TAGDAEMON_H

#include "daemonplugin_tag_global.h"

#include <dfm-framework/dpf.h>

class TagManagerDBus;

DAEMONPTAG_BEGIN_NAMESPACE

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
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "tagdaemon.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requestLaunch();

private:
    QThread workerThread;
};

DAEMONPTAG_END_NAMESPACE

#endif   // TAGDAEMON_H
