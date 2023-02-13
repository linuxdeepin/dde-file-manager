// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACE_H
#define DETAILSPACE_H

#include "dfmplugin_detailspace_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_detailspace {

class DetailSpace : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "detailspace.json")

    DPF_EVENT_NAMESPACE(DPDETAILSPACE_NAMESPACE)

    // slot events
    DPF_EVENT_REG_SLOT(slot_DetailView_Show)
    DPF_EVENT_REG_SLOT(slot_ViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_BasicViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_BasicViewExtension_Root_Register)
    DPF_EVENT_REG_SLOT(slot_BasicFiledFilter_Add)
    DPF_EVENT_REG_SLOT(slot_BasicFiledFilter_Root_Add)

    // hook evetns
    DPF_EVENT_REG_HOOK(hook_Icon_Fetch)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowClosed(quint64 windId);
};

}

#endif   // DETAILSPACE_H
