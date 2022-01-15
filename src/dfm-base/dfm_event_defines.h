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

class QUrl;
DFMBASE_BEGIN_NAMESPACE

/*!
 * \brief The PublicEventType enum define Event type
 * that can be used in multiple plugins
 */
enum GlobalEventType {
    kUnknowType = 0,

    kChangeCurrentUrl = 1,
    kOpenNewWindow,
    kOpenNewTab,
    kOpenUrl,
    kSaveOperator,   // save operator
    kCleanSaveOperator,   // Ctrl+Z
    kRevocation,
    // file operations
    kOpenFiles,
    kOpenFilesByApp,
    kRenameFile,
    kMkdir,
    kMkdirCallBack,
    kTouchFile,
    kTouchCallBack,
    kCreateSymlink,
    kSetPermission,
    kCopy,
    kDeleteFiles,
    kMoveToTrash,
    kRestoreFromTrash,
    kPasteFile,
    kCutFile,
    kOpenFilesResult,
    kOpenFilesByAppResult,
    kRenameFileResult,
    kMkdirResult,
    kTouchFileResult,
    kCreateSymlinkResult,
    kSetPermissionResult,

    // first user event id, use UniversalUtils::registerEventType crreate custom event type
    kCustomBase = 1000,   // first user event id
    kMaxCustom = 65535   // last user event id
};
DFMBASE_END_NAMESPACE
#endif   // DFM_EVENT_DEFINES_H
