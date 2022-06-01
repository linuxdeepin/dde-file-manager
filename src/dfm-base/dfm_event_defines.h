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
#ifndef DFM_EVENT_DEFINES_H
#define DFM_EVENT_DEFINES_H

#include "dfm_base_global.h"

#include <QObject>

#include <dfm-framework/event/event.h>

DFMBASE_BEGIN_NAMESPACE

/*!
 * \brief The PublicEventType enum define Event type
 * that can be used in multiple plugins
 */
enum GlobalEventType : DPF_NAMESPACE::EventType {
    kUnknowType = DPF_NAMESPACE::EventTypeScope::kWellKnownEventBase,

    kChangeCurrentUrl = 1,
    kOpenNewWindow,
    kOpenNewTab,
    kSaveOperator,   // save operator
    kCleanSaveOperator,   // Ctrl+Z
    kRevocation,
    kOpenAsAdmin,
    kSwitchViewMode,

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
    kDeleteFiles,
    kMoveToTrash,
    kRestoreFromTrash,
    kWriteUrlsToClipboard,
    kCutFile,
    kOpenInTerminal,
    kCleanTrash,
    kWriteCustomToClipboard,
    kHideFiles,

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

    kTempDesktopPaintTag = 9998,   //! todo(zy) need to delete when find solution

    kMaxEventType = DPF_NAMESPACE::EventTypeScope::kWellKnownEventTop
};
DFMBASE_END_NAMESPACE

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::GlobalEventType)

#endif   // DFM_EVENT_DEFINES_H
