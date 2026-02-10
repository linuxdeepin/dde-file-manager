// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERPLUGIN_H
#define ORGANIZERPLUGIN_H

#include "ddplugin_organizer_global.h"

#include <dfm-framework/dpf.h>

namespace ddplugin_organizer {

class FrameManager;
class OrganizerPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "organizerplugin.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    void bindEvent();

    FrameManager *instance = nullptr;

private:
    DPF_EVENT_NAMESPACE(DDP_ORGANIZER_NAMESPACE)

    // Organizer Common
    DPF_EVENT_REG_SLOT(slot_Organizer_Enabled)

    // CollectionView begin
    DPF_EVENT_REG_SLOT(slot_CollectionView_GridPoint)
    DPF_EVENT_REG_SLOT(slot_CollectionView_VisualRect)
    DPF_EVENT_REG_SLOT(slot_CollectionView_View)
    DPF_EVENT_REG_SLOT(slot_CollectionModel_SelectAll)

    DPF_EVENT_REG_SIGNAL(signal_CollectionView_ReportMenuData)

    DPF_EVENT_REG_HOOK(hook_CollectionView_DropData)
    DPF_EVENT_REG_HOOK(hook_CollectionView_KeyPress)
    DPF_EVENT_REG_HOOK(hook_CollectionView_StartDrag)
    DPF_EVENT_REG_HOOK(hook_CollectionView_DragMove)
    DPF_EVENT_REG_HOOK(hook_CollectionView_KeyboardSearch)
    DPF_EVENT_REG_HOOK(hook_CollectionView_DrawFile)
    DPF_EVENT_REG_HOOK(hook_CollectionView_ShortcutKeyPress)

    // CollectionItemDelegate begin
    DPF_EVENT_REG_SLOT(slot_CollectionItemDelegate_IconRect)

    // CollectionModel begin
    DPF_EVENT_REG_SLOT(slot_CollectionModel_Refresh)
};

}

#endif   // ORGANIZERPLUGIN_H
