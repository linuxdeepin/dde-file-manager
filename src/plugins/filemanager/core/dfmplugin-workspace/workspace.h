// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    DPF_EVENT_REG_SIGNAL(signal_Model_EmptyDir)

    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_Commit)

    // slot events
    DPF_EVENT_REG_SLOT(slot_RegisterFileView)
    DPF_EVENT_REG_SLOT(slot_RegisterMenuScene)
    DPF_EVENT_REG_SLOT(slot_FindMenuScene)
    DPF_EVENT_REG_SLOT(slot_RegisterCustomTopWidget)
    DPF_EVENT_REG_SLOT(slot_ShowCustomTopWidget)
    DPF_EVENT_REG_SLOT(slot_GetCustomTopWidgetVisible)
    DPF_EVENT_REG_SLOT(slot_CheckSchemeViewIsFileView)
    DPF_EVENT_REG_SLOT(slot_RefreshDir)
    DPF_EVENT_REG_SLOT(slot_NotSupportTreeView)

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
    DPF_EVENT_REG_SLOT(slot_View_ReverseSelect)
    DPF_EVENT_REG_SLOT(slot_View_SetSelectionMode)
    DPF_EVENT_REG_SLOT(slot_View_SetEnabledSelectionModes)
    DPF_EVENT_REG_SLOT(slot_View_SetDragEnabled)
    DPF_EVENT_REG_SLOT(slot_View_SetDragDropMode)
    DPF_EVENT_REG_SLOT(slot_View_ClosePersistentEditor)
    DPF_EVENT_REG_SLOT(slot_View_SetReadOnly)
    DPF_EVENT_REG_SLOT(slot_View_SetFilter)
    DPF_EVENT_REG_SLOT(slot_View_GetFilter)
    DPF_EVENT_REG_SLOT(slot_View_SetAlwaysOpenInCurrentWindow)

    DPF_EVENT_REG_SLOT(slot_Model_SetCustomFilterData)
    DPF_EVENT_REG_SLOT(slot_Model_SetCustomFilterCallback)
    DPF_EVENT_REG_SLOT(slot_Model_RegisterRoutePrehandle)
    DPF_EVENT_REG_SLOT(slot_Model_SetNameFilter)
    DPF_EVENT_REG_SLOT(slot_Model_GetNameFilter)
    DPF_EVENT_REG_SLOT(slot_Model_FileUpdate)
    DPF_EVENT_REG_SLOT(slot_Model_CurrentSortRole)
    DPF_EVENT_REG_SLOT(slot_Model_ColumnRoles)
    DPF_EVENT_REG_SLOT(slot_Model_SetSort)
    DPF_EVENT_REG_SLOT(slot_Model_RegisterDataCache)

    // hook events
    DPF_EVENT_REG_HOOK(hook_SendOpenWindow)
    DPF_EVENT_REG_HOOK(hook_SendChangeCurrentUrl)

    DPF_EVENT_REG_HOOK(hook_Tab_SetTabName)
    DPF_EVENT_REG_HOOK(hook_Tab_Closeable)

    DPF_EVENT_REG_HOOK(hook_DragDrop_CheckDragDropAction)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileDragMove)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileDrop)
    DPF_EVENT_REG_HOOK(hook_DragDrop_IsDrop)
    DPF_EVENT_REG_HOOK(hook_DragDrop_FileCanMove)

    DPF_EVENT_REG_HOOK(hook_ShortCut_CopyFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_CutFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_PasteFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_DeleteFiles)
    DPF_EVENT_REG_HOOK(hook_ShortCut_MoveToTrash)
    DPF_EVENT_REG_HOOK(hook_ShortCut_EnterPressed)
    DPF_EVENT_REG_HOOK(hook_ShortCut_PreViewFiles)

    DPF_EVENT_REG_HOOK(hook_Delegate_PaintListItem)
    DPF_EVENT_REG_HOOK(hook_Delegate_LayoutText)
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

private:
    void registerQmlType();
};

}

#endif   // WORKSPACE_H
