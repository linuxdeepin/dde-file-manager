// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Auto-generated helper: registers ALL hook event types used by every
// plugin followEvents() call. Normally these are registered at static-init
// by the *owning* plugins DPF_EVENT_REG_HOOK members, but those plugin libs
// are not linked into per-plugin test binaries. Calling this before running
// a real followEvents()/bindEvents() lets the subscribe templates execute.
#pragma once
#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>

namespace dfmtest_hooks {
inline void registerAllHookEvents(dpf::Event *evt = dpf::Event::instance()) {
    using S = dpf::EventStratege;
    evt->registerEventType(S::kHook, "ddplugin_canvas", "hook_CanvasItemDelegate_LayoutText");
    evt->registerEventType(S::kHook, "ddplugin_canvas", "hook_CanvasView_DragMove");
    evt->registerEventType(S::kHook, "ddplugin_canvas", "hook_CanvasView_DropData");
    evt->registerEventType(S::kHook, "ddplugin_core", "hook_ScreenProxy_ScreensInUse");
    evt->registerEventType(S::kHook, "ddplugin_organizer", "hook_CollectionView_DragMove");
    evt->registerEventType(S::kHook, "ddplugin_organizer", "hook_CollectionView_DropData");
    evt->registerEventType(S::kHook, "dfmplugin_computer", "hook_Device_AcquireDevPwd");
    evt->registerEventType(S::kHook, "dfmplugin_detailspace", "hook_Icon_Fetch");
    evt->registerEventType(S::kHook, "dfmplugin_emblem", "hook_ExtendEmblems_Fetch");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_OpenLocalFiles");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CopyFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CopyFromFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CutFromFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_CutToFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_DeleteFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_MakeDir");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_MoveToTrash");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_OpenInTerminal");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_RenameFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_RenameFiles");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_RenameFilesAddText");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_SetPermission");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_TouchCustomFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_TouchFile");
    evt->registerEventType(S::kHook, "dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard");
    evt->registerEventType(S::kHook, "dfmplugin_propertydialog", "hook_PermissionView_Ash");
    evt->registerEventType(S::kHook, "dfmplugin_propertydialog", "hook_PropertyDialog_Disable");
    evt->registerEventType(S::kHook, "dfmplugin_search", "hook_Url_IsSubFile");
    evt->registerEventType(S::kHook, "dfmplugin_sidebar", "hook_Group_Sort");
    evt->registerEventType(S::kHook, "dfmplugin_sidebar", "hook_Item_DragMoveData");
    evt->registerEventType(S::kHook, "dfmplugin_sidebar", "hook_Item_DropData");
    evt->registerEventType(S::kHook, "dfmplugin_tag", "hook_CanTaged");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Copy_Addr");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Crumb_RedirectUrl");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Crumb_Seprate");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Show_Addr");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Tab_Closeable");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Tab_EnterDir");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Tab_FileDeleteNotCdComputer");
    evt->registerEventType(S::kHook, "dfmplugin_titlebar", "hook_Tab_SetTabName");
    evt->registerEventType(S::kHook, "dfmplugin_utils", "hook_AppendCompress_Prohibit");
    evt->registerEventType(S::kHook, "dfmplugin_utils", "hook_OpenWith_DisabledOpenWithWidget");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Allow_Repeat_Url");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Delegate_CheckTransparent");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Delegate_LayoutText");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Delegate_PaintListItem");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_CheckDragDropAction");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_FileCanMove");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_FileDragMove");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_FileDrop");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_DragDrop_IsDrop");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Model_FetchCustomColumnRoles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Model_FetchCustomRoleDisplayName");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_SendChangeCurrentUrl");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_SendOpenWindow");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_CopyFilePath");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_CopyFiles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_CutFiles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_DeleteFiles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_EnterPressed");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_MoveToTrash");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_PasteFiles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_ShortCut_PreViewFiles");
    evt->registerEventType(S::kHook, "dfmplugin_workspace", "hook_Url_FetchPathtoVirtual");
}
} // namespace dfmtest_hooks
