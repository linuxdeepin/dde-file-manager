/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_workspace {

class Workspace : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "workspace.json")

    DPF_EVENT_NAMESPACE(DPWORKSPACE_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_Tab_Added)
    DPF_EVENT_REG_SIGNAL(signal_Tab_Changed)
    DPF_EVENT_REG_SIGNAL(signal_Tab_Moved)
    DPF_EVENT_REG_SIGNAL(signal_Tab_Removed)

    DPF_EVENT_REG_SIGNAL(signal_View_SelectionChanged)
    DPF_EVENT_REG_SIGNAL(signal_View_RenameStartEdit)
    DPF_EVENT_REG_SIGNAL(signal_View_RenameEndEdit)
    DPF_EVENT_REG_SIGNAL(signal_View_ItemClicked)
    DPF_EVENT_REG_SIGNAL(signal_View_HeaderViewSectionChanged)

    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

    // slot events
    DPF_EVENT_REG_SLOT(slot_RegisterFileView)
    DPF_EVENT_REG_SLOT(slot_RegisterMenuScene)
    DPF_EVENT_REG_SLOT(slot_FindMenuScene)
    DPF_EVENT_REG_SLOT(slot_RegisterCustomTopWidget)
    DPF_EVENT_REG_SLOT(slot_ShowCustomTopWidget)
    DPF_EVENT_REG_SLOT(slot_GetCustomTopWidgetVisible)
    DPF_EVENT_REG_SLOT(slot_CheckSchemeViewIsFileView)

    DPF_EVENT_REG_SLOT(slot_Tab_Addable)
    DPF_EVENT_REG_SLOT(slot_Tab_Close)
    DPF_EVENT_REG_SLOT(slot_Tab_SetAlias)

    DPF_EVENT_REG_SLOT(slot_View_GetVisualGeometry)
    DPF_EVENT_REG_SLOT(slot_View_GetViewItemRect)
    DPF_EVENT_REG_SLOT(slot_View_GetCurrentViewMode)
    DPF_EVENT_REG_SLOT(slot_View_GetDefaultViewMode)
    DPF_EVENT_REG_SLOT(slot_View_SetDefaultViewMode)
    DPF_EVENT_REG_SLOT(slot_View_GetSelectedUrls)
    DPF_EVENT_REG_SLOT(slot_View_SelectFiles)
    DPF_EVENT_REG_SLOT(slot_View_SelectAll)
    DPF_EVENT_REG_SLOT(slot_View_SetSelectionMode)
    DPF_EVENT_REG_SLOT(slot_View_SetEnabledSelectionModes)
    DPF_EVENT_REG_SLOT(slot_View_SetDragEnabled)
    DPF_EVENT_REG_SLOT(slot_View_SetDragDropMode)
    DPF_EVENT_REG_SLOT(slot_View_ClosePersistentEditor)
    DPF_EVENT_REG_SLOT(slot_View_SetReadOnly)
    DPF_EVENT_REG_SLOT(slot_View_SetFilter)
    DPF_EVENT_REG_SLOT(slot_View_GetFilter)

    DPF_EVENT_REG_SLOT(slot_Model_SetCustomFilterData)
    DPF_EVENT_REG_SLOT(slot_Model_SetCustomFilterCallback)
    DPF_EVENT_REG_SLOT(slot_Model_RegisterRoutePrehandle)
    DPF_EVENT_REG_SLOT(slot_Model_SetNameFilter)
    DPF_EVENT_REG_SLOT(slot_Model_GetNameFilter)
    DPF_EVENT_REG_SLOT(slot_Model_FileUpdate)
    DPF_EVENT_REG_SLOT(slot_Model_CurrentSortRole)
    DPF_EVENT_REG_SLOT(slot_Model_SetSort)
    DPF_EVENT_REG_SLOT(slot_Model_RegisterDataCache)

    // hook events
    DPF_EVENT_REG_HOOK(hook_SendOpenWindow)
    DPF_EVENT_REG_HOOK(hook_SendChangeCurrentUrl)

    DPF_EVENT_REG_HOOK(hook_Tab_SetTabName)

    DPF_EVENT_REG_HOOK(hook_DragDrop_CheckDragDropAction)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileDragMove)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileDrop)
    DPF_EVENT_REG_HOOK(hook_DragDrop_IsDrop)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileCanMove)

    DPF_EVENT_REG_HOOK(hook_ShortCut_PasteFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_DeleteFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_MoveToTrash)
    DPF_EVENT_REG_HOOK(hook_ShortCut_EnterPressed)

    DPF_EVENT_REG_HOOK(hook_Delegate_PaintListItem)
    DPF_EVENT_REG_HOOK(hook_Delegate_PaintIconItem)
    DPF_EVENT_REG_HOOK(hook_Delegate_CheckTransparent)

    DPF_EVENT_REG_HOOK(hook_View_FetchSupportSelectionModes)

    DPF_EVENT_REG_HOOK(hook_Model_FetchCustomColumnRoles)
    DPF_EVENT_REG_HOOK(hook_Model_FetchCustomRoleDisplayName)

    DPF_EVENT_REG_HOOK(hook_Url_FetchPathtoVirtual)

public:
    virtual void initialize() override;
    virtual bool start() override;

signals:
    void readyToInstallWidget(const quint64);

private slots:
    void onWindowOpened(quint64 windId);
    void onWindowClosed(quint64 winId);
};

}

#endif   // WORKSPACE_H
