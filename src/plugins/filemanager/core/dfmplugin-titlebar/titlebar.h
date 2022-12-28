/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

    // slot events
    DPF_EVENT_REG_SLOT(slot_Custom_Register)
    DPF_EVENT_REG_SLOT(slot_Spinner_Start)
    DPF_EVENT_REG_SLOT(slot_Spinner_Stop)
    DPF_EVENT_REG_SLOT(slot_FilterButton_Show)
    DPF_EVENT_REG_SLOT(slot_NewWindowAndTab_SetEnable)
    DPF_EVENT_REG_SLOT(slot_Navigator_Forward)
    DPF_EVENT_REG_SLOT(slot_Navigator_Backward)
    DPF_EVENT_REG_SLOT(slot_SharePasswordSettingsDialog_Show)
    DPF_EVENT_REG_SLOT(slot_ServerDialog_RemoveHistory)

    // hook events
    DPF_EVENT_REG_HOOK(hook_Crumb_Seprate)

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
