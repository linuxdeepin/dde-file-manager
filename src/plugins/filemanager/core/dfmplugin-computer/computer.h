/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
    DPF_EVENT_REG_SLOT(slot_AddDevice)
    DPF_EVENT_REG_SLOT(slot_RemoveDevice)
    DPF_EVENT_REG_SLOT(slot_ComputerView_Refresh) // TODO(xust) impl it.
    
    // signals
    DPF_EVENT_REG_SIGNAL(signal_Operation_OpenItem)
    DPF_EVENT_REG_SIGNAL(signal_ShortCut_CtrlN)
    DPF_EVENT_REG_SIGNAL(signal_ShortCut_CtrlT)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

    // hook
    DPF_EVENT_REG_HOOK(hook_ComputerView_ItemListFilter)
    DPF_EVENT_REG_HOOK(hook_ComputerView_ItemFilterOnAdd)
    DPF_EVENT_REG_HOOK(hook_ComputerView_ItemFilterOnRemove)

    // Plugin interface
public:
    virtual void initialize() override;
    virtual bool start() override;

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);

private:
    void addComputerToSidebar();
    void regComputerCrumbToTitleBar();
    void regComputerToSearch();
    void bindEvents();
    void followEvents();
    void bindWindows();
};
}

#endif   // COMPUTER_H
