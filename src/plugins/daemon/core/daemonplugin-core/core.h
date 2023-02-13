// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORE_H
#define CORE_H

#include "daemonplugin_core_global.h"

#include <dfm-framework/dpf.h>

DAEMONPCORE_BEGIN_NAMESPACE

class Core : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "core.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
};

DAEMONPCORE_END_NAMESPACE

#endif   // CORE_H
