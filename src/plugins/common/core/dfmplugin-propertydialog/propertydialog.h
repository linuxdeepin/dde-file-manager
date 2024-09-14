// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACE_H
#define DETAILSPACE_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_propertydialog {

class PropertyDialog : public dpf::Plugin
{
    Q_OBJECT
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "propertydialog.json")
#else
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common.qt6" FILE "propertydialog.json")
#endif

    DPF_EVENT_NAMESPACE(DPPROPERTYDIALOG_NAMESPACE)

    // slot events
    DPF_EVENT_REG_SLOT(slot_PropertyDialog_Show)
    DPF_EVENT_REG_SLOT(slot_ViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_CustomView_Register)
    DPF_EVENT_REG_SLOT(slot_BasicViewExtension_Register)
    DPF_EVENT_REG_SLOT(slot_BasicFiledFilter_Add)

    // hook events
    DPF_EVENT_REG_HOOK(hook_PermissionView_Ash)
    DPF_EVENT_REG_HOOK(hook_PropertyDialog_Disable)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // DETAILSPACE_H
