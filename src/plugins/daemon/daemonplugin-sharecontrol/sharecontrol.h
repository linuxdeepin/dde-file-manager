// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROL_H
#define SHARECONTROL_H

#include "daemonplugin_sharecontrol_global.h"

#include <dfm-framework/dpf.h>

class ShareControlDBus;
DAEMONPSHARECONTROL_BEGIN_NAMESPACE

class ShareControl : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "sharecontrol.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<ShareControlDBus> mng;
};

DAEMONPSHARECONTROL_END_NAMESPACE
#endif   // SHARECONTROL_H
