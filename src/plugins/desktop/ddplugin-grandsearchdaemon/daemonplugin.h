// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_H
#define DAEMONPLUGIN_H

#include "ddplugin_grandsearchdaemon_global.h"

#include <dfm-framework/dpf.h>

DDP_GRANDSEARCHDAEMON_BEGIN_NAMESPACE
class DaemonLibrary;
class DaemonPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "grandsearchdaemon.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;
private:
    DaemonLibrary *library = nullptr;
};

DDP_GRANDSEARCHDAEMON_END_NAMESPACE

#endif // DAEMONPLUGIN_H
