// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASPLUGIN_H
#define CANVASPLUGIN_H

#include "ddplugin_canvas_global.h"

#include <dfm-framework/dpf.h>

namespace ddplugin_canvas {
class CanvasManager;
class CanvasPlugin : public dpf::Plugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "canvas.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    void registerDBus();

private:
    CanvasManager *proxy = nullptr;

private:
    DPF_EVENT_NAMESPACE(DDP_CANVAS_NAMESPACE)
    // CanvasManager begin
    DPF_EVENT_REG_SIGNAL(signal_CanvasManager_IconSizeChanged)
    DPF_EVENT_REG_SIGNAL(signal_CanvasManager_FontChanged)
    DPF_EVENT_REG_SIGNAL(signal_CanvasManager_AutoArrangeChanged)

    DPF_EVENT_REG_SLOT(slot_CanvasManager_FileInfoModel)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_Update)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_Edit)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_IconLevel)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_SetIconLevel)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_AutoArrange)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_SetAutoArrange)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_View)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_SelectionModel)

    DPF_EVENT_REG_HOOK(hook_CanvasManager_RequestWallpaperSetting)

    // CanvasView begin
    DPF_EVENT_REG_SIGNAL(signal_CanvasView_RequestRefresh)
    DPF_EVENT_REG_SLOT(slot_CanvasView_VisualRect)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Refresh)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Update)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Select)
    DPF_EVENT_REG_SLOT(slot_CanvasView_SelectedUrls)
    DPF_EVENT_REG_SLOT(slot_CanvasView_GridSize)
    DPF_EVENT_REG_SLOT(slot_CanvasView_GridPos)
    DPF_EVENT_REG_SLOT(slot_CanvasView_GridVisualRect)
    //private slot
    DPF_EVENT_REG_SLOT(slot_CanvasViewPrivate_FileOperator)

    DPF_EVENT_REG_HOOK(hook_CanvasView_ContextMenu)
    DPF_EVENT_REG_HOOK(hook_CanvasView_DropData)
    DPF_EVENT_REG_HOOK(hook_CanvasView_KeyPress)
    DPF_EVENT_REG_HOOK(hook_CanvasView_MousePress)
    DPF_EVENT_REG_HOOK(hook_CanvasView_MouseRelease)
    DPF_EVENT_REG_HOOK(hook_CanvasView_MouseDoubleClick)
    DPF_EVENT_REG_HOOK(hook_CanvasView_Wheel)
    DPF_EVENT_REG_HOOK(hook_CanvasView_StartDrag)
    DPF_EVENT_REG_HOOK(hook_CanvasView_DragEnter)
    DPF_EVENT_REG_HOOK(hook_CanvasView_DragMove)
    DPF_EVENT_REG_HOOK(hook_CanvasView_DragLeave)
    DPF_EVENT_REG_HOOK(hook_CanvasView_KeyboardSearch)
    DPF_EVENT_REG_HOOK(hook_CanvasView_DrawFile)
    DPF_EVENT_REG_HOOK(hook_CanvasView_ShortcutKeyPress)

    DPF_EVENT_REG_SIGNAL(signal_CanvasView_ReportMenuData)

    // CanvasGrid begin
    DPF_EVENT_REG_SLOT(slot_CanvasGrid_Items)
    DPF_EVENT_REG_SLOT(slot_CanvasGrid_Item)
    DPF_EVENT_REG_SLOT(slot_CanvasGrid_Point)
    DPF_EVENT_REG_SLOT(slot_CanvasGrid_TryAppendAfter)

    // CanvasModel begin
    DPF_EVENT_REG_SIGNAL(signal_CanvasModel_HiddenFlagChanged)

    DPF_EVENT_REG_SLOT(slot_CanvasModel_RootUrl)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_UrlIndex)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Index)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_FileUrl)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Files)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_ShowHiddenFiles)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_SetShowHiddenFiles)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_SortOrder)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_SetSortOrder)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_SortRole)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_SetSortRole)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_RowCount)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Data)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Sort)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Refresh)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Fetch)
    DPF_EVENT_REG_SLOT(slot_CanvasModel_Take)

    DPF_EVENT_REG_HOOK(hook_CanvasModel_Data)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DataInserted)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DataRemoved)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DataRenamed)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DataRested)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DataChanged)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_DropMimeData)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_MimeData)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_MimeTypes)
    DPF_EVENT_REG_HOOK(hook_CanvasModel_SortData)

    // FileInfoModel begin
    DPF_EVENT_REG_SIGNAL(signal_FileInfoModel_DataReplaced)
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_LoadFilesFinish)

    DPF_EVENT_REG_SLOT(slot_FileInfoModel_RootUrl)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_RootIndex)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_UrlIndex)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_IndexUrl)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_Files)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_FileInfo)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_Refresh)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_ModelState)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_UpdateFile)

    // CanvasItemDelegate begin
    DPF_EVENT_REG_SLOT(slot_CanvasItemDelegate_IconRect)

    DPF_EVENT_REG_HOOK(hook_CanvasItemDelegate_LayoutText)

    // CanvasSelectionModel begin
    DPF_EVENT_REG_SIGNAL(signal_CanvasSelectionModel_Clear)
};

}

#endif   // CANVASPLUGIN_H
