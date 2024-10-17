// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_titlebar {

class TitleBar : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "titlebar.json")

    DPF_EVENT_NAMESPACE(DPTITLEBAR_NAMESPACE)
    // singnal events
    DPF_EVENT_REG_SIGNAL(signal_Search_Start)
    DPF_EVENT_REG_SIGNAL(signal_Search_Stop)
    DPF_EVENT_REG_SIGNAL(signal_FilterView_Show)
    DPF_EVENT_REG_SIGNAL(signal_InputAdddressStr_Check)
    DPF_EVENT_REG_SIGNAL(signal_Share_SetPassword)

    // slot events
    DPF_EVENT_REG_SLOT(slot_Custom_Register)
    DPF_EVENT_REG_SLOT(slot_Spinner_Start)
    DPF_EVENT_REG_SLOT(slot_Spinner_Stop)
    DPF_EVENT_REG_SLOT(slot_FilterButton_Show)
    DPF_EVENT_REG_SLOT(slot_NewWindowAndTab_SetEnable)
    DPF_EVENT_REG_SLOT(slot_Navigator_Forward)
    DPF_EVENT_REG_SLOT(slot_Navigator_Backward)
    DPF_EVENT_REG_SLOT(slot_Navigator_Remove)
    DPF_EVENT_REG_SLOT(slot_ServerDialog_RemoveHistory)

    DPF_EVENT_REG_SLOT(slot_Tab_Addable)
    DPF_EVENT_REG_SLOT(slot_Tab_Close)
    DPF_EVENT_REG_SLOT(slot_Tab_SetAlias)

    // hook events
    DPF_EVENT_REG_HOOK(hook_Crumb_Seprate)
    DPF_EVENT_REG_HOOK(hook_Show_Addr)
    DPF_EVENT_REG_HOOK(hook_Copy_Addr)

    DPF_EVENT_REG_HOOK(hook_Tab_SetTabName)
    DPF_EVENT_REG_HOOK(hook_Tab_Closeable)
    DPF_EVENT_REG_HOOK(hook_Tab_FileDeleteNotCdComputer)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowCreated(quint64 windId);
    void onWindowOpened(quint64 windId);
    void onWindowClosed(quint64 windId);
    void bindEvents();
};

}

#endif   // TITLEBAR_H
