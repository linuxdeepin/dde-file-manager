// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALSHARE_H
#define OPTICALSHARE_H

#include "daemonplugin_opticalshare_global.h"

#include <dfm-framework/dpf.h>

class OpticalShareDBus;

DAEMONPOPTICALSHARE_BEGIN_NAMESPACE

class OpticalShareDBusWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void launchService();

private:
    QScopedPointer<OpticalShareDBus> opticalShare;
};

class OpticalShare : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "opticalshare.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requestLaunch();

private:
    QThread workerThread;
};

DAEMONPOPTICALSHARE_END_NAMESPACE

#endif   // OPTICALSHARE_H
