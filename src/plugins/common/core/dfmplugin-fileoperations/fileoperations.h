// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATIONSPLUGIN_H
#define FILEOPERATIONSPLUGIN_H

#include "dfmplugin_fileoperations_global.h"

#include <dfm-framework/dpf.h>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileOperations : public dpf::Plugin
{
    Q_OBJECT
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "fileoperations.json")
#else
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common.qt6" FILE "fileoperations.json")
#endif

    DPF_EVENT_NAMESPACE(DPFILEOPERATIONS_NAMESPACE)

    // hook events
    DPF_EVENT_REG_HOOK(hook_OpenLocalFiles)

    // hook events -- file operation
    DPF_EVENT_REG_HOOK(hook_Operation_CopyFile)
    DPF_EVENT_REG_HOOK(hook_Operation_CopyFromFile)
    DPF_EVENT_REG_HOOK(hook_Operation_CutToFile)
    DPF_EVENT_REG_HOOK(hook_Operation_CutFromFile)
    DPF_EVENT_REG_HOOK(hook_Operation_DeleteFile)
    DPF_EVENT_REG_HOOK(hook_Operation_MoveToTrash)
    DPF_EVENT_REG_HOOK(hook_Operation_RestoreFromTrash)
    DPF_EVENT_REG_HOOK(hook_Operation_OpenFileInPlugin)
    DPF_EVENT_REG_HOOK(hook_Operation_OpenFileByApp)
    DPF_EVENT_REG_HOOK(hook_Operation_OpenInTerminal)
    DPF_EVENT_REG_HOOK(hook_Operation_RenameFile)
    DPF_EVENT_REG_HOOK(hook_Operation_MakeDir)
    DPF_EVENT_REG_HOOK(hook_Operation_TouchFile)
    DPF_EVENT_REG_HOOK(hook_Operation_TouchCustomFile)
    DPF_EVENT_REG_HOOK(hook_Operation_LinkFile)
    DPF_EVENT_REG_HOOK(hook_Operation_SetPermission)
    DPF_EVENT_REG_HOOK(hook_Operation_WriteUrlsToClipboard)
    DPF_EVENT_REG_HOOK(hook_Operation_RenameFiles)
    DPF_EVENT_REG_HOOK(hook_Operation_RenameFilesAddText)
    DPF_EVENT_REG_SLOT(slot_Operation_FilesPreview)

    DPF_EVENT_REG_SIGNAL(signal_File_Add)
    DPF_EVENT_REG_SIGNAL(signal_File_Delete)
    DPF_EVENT_REG_SIGNAL(signal_File_Rename)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void initEventHandle();

private:
    void followEvents();
};

DPFILEOPERATIONS_END_NAMESPACE
#endif   // COREPLUGIN_H
