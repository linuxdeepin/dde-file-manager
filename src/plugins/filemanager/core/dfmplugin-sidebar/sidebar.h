// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "dfmplugin_sidebar_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_sidebar {

class SideBarWidget;
class SideBar : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "sidebar.json")

    DPF_EVENT_NAMESPACE(DPSIDEBAR_NAMESPACE)

    // slot events
    DPF_EVENT_REG_SLOT(slot_ContextMenu_SetEnable)   // TODO(xust) tmp solution, using GroupPolicy instead.
    DPF_EVENT_REG_SLOT(slot_Group_UrlList)
    DPF_EVENT_REG_SLOT(slot_Item_Add)
    DPF_EVENT_REG_SLOT(slot_Item_Remove)
    DPF_EVENT_REG_SLOT(slot_Item_Update)
    DPF_EVENT_REG_SLOT(slot_Item_Insert)
    DPF_EVENT_REG_SLOT(slot_Item_Hidden)
    DPF_EVENT_REG_SLOT(slot_Item_TriggerEdit)
    DPF_EVENT_REG_SLOT(slot_Sidebar_UpdateSelection)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_Sidebar_Sorted)
    DPF_EVENT_REG_SIGNAL(signal_Item_EjectClicked)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_Commit)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

    // hook events
    DPF_EVENT_REG_HOOK(hook_Group_Sort)
    DPF_EVENT_REG_HOOK(hook_Item_DropData)
    DPF_EVENT_REG_HOOK(hook_Item_DragMoveData)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void onWindowClosed(quint64 winId);
    void onConfigChanged(const QString &cfg, const QString &key);
    bool onAboutToShowSettingDialog(quint64 winId);

private:
    void initPreDefineItems();
};

}   // namespace dfmplugin_sidebar

#endif   // SIDEBAR_H
