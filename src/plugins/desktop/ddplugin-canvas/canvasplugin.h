/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CANVASPLUGIN_H
#define CANVASPLUGIN_H

#include "ddplugin_canvas_global.h"

#include <dfm-framework/dpf.h>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasManager;
class CanvasPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "canvas.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private:
    CanvasManager *proxy = nullptr;
private:
    DPF_EVENT_NAMESPACE(DDP_CANVAS_NAMESPACE)
    // CanvasManager begin
    DPF_EVENT_REG_SIGNAL(signal_CanvasManager_IconSizeChanged)
    DPF_EVENT_REG_SIGNAL(signal_CanvasManager_HiddenFlagChanged)

    DPF_EVENT_REG_SLOT(slot_CanvasManager_FileInfoModel)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_Update)
    DPF_EVENT_REG_SLOT(slot_CanvasManager_Edit)

    DPF_EVENT_REG_HOOK(hook_CanvasManager_RequestWallpaperSetting)

    // CanvasView begin
    DPF_EVENT_REG_SLOT(slot_CanvasView_VisualRect)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Refresh)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Update)
    DPF_EVENT_REG_SLOT(slot_CanvasView_Select)
    DPF_EVENT_REG_SLOT(slot_CanvasView_SelectedUrls)

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

    // CanvasGrid begin
    DPF_EVENT_REG_SLOT(slot_CanvasGrid_Items)

    // CanvasModel begin
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

    DPF_EVENT_REG_SLOT(slot_FileInfoModel_RootUrl)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_RootIndex)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_UrlIndex)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_IndexUrl)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_Files)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_FileInfo)
    DPF_EVENT_REG_SLOT(slot_FileInfoModel_Refresh)
};

DDP_CANVAS_END_NAMESPACE

#endif   // CANVASPLUGIN_H
