// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEM_H
#define EMBLEM_H

#include "dfmplugin_emblem_global.h"

#include <dfm-framework/dpf.h>

DPEMBLEM_BEGIN_NAMESPACE

class Emblem : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "emblem.json")

    DPF_EVENT_NAMESPACE(DPEMBLEM_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_FileEmblems_Paint)

    DPF_EVENT_REG_HOOK(hook_CustomEmblems_Fetch)
    DPF_EVENT_REG_HOOK(hook_ExtendEmblems_Fetch)

public:
    virtual void initialize() override;
    virtual bool start() override;
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEM_H
