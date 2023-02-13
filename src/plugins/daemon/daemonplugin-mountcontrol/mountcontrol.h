// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROL_H
#define MOUNTCONTROL_H

#include "daemonplugin_mountcontrol_global.h"

#include <dfm-framework/dpf.h>

class MountControlDBus;
DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE

class MountControl : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "mountcontrol.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<MountControlDBus> mng;
};

DAEMONPMOUNTCONTROL_END_NAMESPACE
#endif   // MOUNTCONTROL_H
