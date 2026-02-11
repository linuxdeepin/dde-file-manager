// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_EVENT_DEFINES_H
#define DFM_EVENT_DEFINES_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

namespace dfmbase {

/*!
 * \brief The PublicEventType enum define Event type
 * that can be used in multiple plugins
 */
enum GlobalEventType {
    kUnknowType = 0,   // DPF_NAMESPACE::EventTypeScope::kWellKnownEventBase

    kChangeCurrentUrl = 1, // req url changing
    kOpenNewWindow,
    kOpenNewTab,
    kSaveOperator,   // save operator
    kCleanSaveOperator,   // Ctrl+Z
    kRevocation,
    kOpenAsAdmin,
    kSwitchViewMode,
    kLoadPlugins,
    kHeadlessStarted,
    kShowSettingDialog,
    kSaveRedoOperator,   // save  Ctrl+Z operator
    kCleanSaveOperatorByUrls, // if file delete,clear all ops which contains url
    kRedo,    // Ctrl+Y

    // request file operations
    kOpenFiles = 200,
    kOpenFilesByApp,
    kRenameFile,
    kRenameFiles,
    kMkdir,
    kTouchFile,
    kCreateSymlink,
    kSetPermission,
    kCopy,
    kCopyFromTrash,
    kDeleteFiles,
    kMoveToTrash,
    kRestoreFromTrash,
    kWriteUrlsToClipboard,
    kCutFile,
    kOpenInTerminal,
    kCleanTrash,
    kWriteCustomToClipboard,
    kHideFiles,
    kCopyFilePath,

    // file operations finieshed!
    kOpenFilesResult = 1000,
    kOpenFilesByAppResult,
    kRenameFileResult,   // contains result of kRenameFile and kRenameFiles
    kMkdirResult,
    kTouchFileResult,
    kCreateSymlinkResult,
    kSetPermissionResult,
    kCopyResult,
    kDeleteFilesResult,
    kMoveToTrashResult,
    kRestoreFromTrashResult,
    kCutFileResult,
    kOpenInTerminalResult,
    kCleanTrashResult,
    kHideFilesResult,
    kMaxEventType = 10000,   // DPF_NAMESPACE::EventTypeScope::kWellKnownEventTo
};
}

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::GlobalEventType)

#endif   // DFM_EVENT_DEFINES_H
