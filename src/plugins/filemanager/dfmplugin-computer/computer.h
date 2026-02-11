// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTER_H
#define COMPUTER_H

#include "dfmplugin_computer_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_computer {
class Computer : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "computer.json")

    DPF_EVENT_NAMESPACE(DPCOMPUTER_NAMESPACE)
    // slots
    DPF_EVENT_REG_SLOT(slot_ContextMenu_SetEnable)   // TODO(xust) tmp solution, using GroupPolicy instead.
    DPF_EVENT_REG_SLOT(slot_Item_Add)
    DPF_EVENT_REG_SLOT(slot_Item_Remove)
    DPF_EVENT_REG_SLOT(slot_View_Refresh)
    DPF_EVENT_REG_SLOT(slot_Passwd_Clear)

    // signals
    DPF_EVENT_REG_SIGNAL(signal_Operation_OpenItem)
    DPF_EVENT_REG_SIGNAL(signal_ShortCut_CtrlN)
    DPF_EVENT_REG_SIGNAL(signal_ShortCut_CtrlT)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)
    DPF_EVENT_REG_SIGNAL(signal_View_Refreshed)
    DPF_EVENT_REG_SIGNAL(signal_Item_Renamed)

    // hook
    DPF_EVENT_REG_HOOK(hook_View_ItemListFilter)
    DPF_EVENT_REG_HOOK(hook_View_ItemFilterOnAdd)
    DPF_EVENT_REG_HOOK(hook_View_ItemFilterOnRemove)
    DPF_EVENT_REG_HOOK(hook_Device_AcquireDevPwd)

    // Plugin interface
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);

private:
    void updateComputerToSidebar();
    void initComputerItems();
    void regComputerCrumbToTitleBar();
    void regComputerToSearch();
    void addComputerSettingItem();
    void bindEvents();
    void followEvents();
    void bindWindows();
};
}

#endif   // COMPUTER_H
