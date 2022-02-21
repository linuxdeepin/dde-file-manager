/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef DFM_ACTIONTYPE_DEFINES_H
#define DFM_ACTIONTYPE_DEFINES_H

#include "dfm_base_global.h"

DFMBASE_BEGIN_NAMESPACE

/*!
 * \brief The PublicActionType enum define action type
 * that can be used in multiple plugins
 */
enum ActionType {
    kUnKnow = 0,
    kActOpen,
    kActNewFolder,
    kActNewDocument,
    kActNewWord,
    kActNewExcel,
    kActNewPowerpoint,
    kActNewText,
    kActOpenInNewWindow,
    kActOpenInNewTab,
    kActDisplayAs,
    kActSortBy,
    kActOpenWith,
    kActOpenAsAdmin,
    kActOpenInTerminal,
    kActSelectAll,
    kActProperty,
    kActCut,
    kActCopy,
    kActPaste,
    kActDelete,
    kActCompleteDeletion,
    kActRename,
    kActRefreshView,
    kActSeparator,
    kActCompress,
    kActCreateSymlink,
    kActSendToDesktop,
    kActSendToBluetooth,
    kActName,
    kActSize,
    kActType,
    kActCreatedDate,
    kActLastModifiedDate,
    kActLastRead,

    // first user actiontype id, use registerActionType create custom event type
    kActCustomBase = 1000,   // first user actiontype id
    kActMaxCustom = 65535   // last user actiontype id
};

enum ExtensionType {
    kTemplateAction = 0x0001,
    kDesktopAction = 0x0002,
    kConfAction = 0x0004,
    kSoAction = 0x0008,
    kAllExtensionAction = kTemplateAction | kDesktopAction | kConfAction | kSoAction,
    kNoExtensionAction = 0x0000,
};

DFMBASE_END_NAMESPACE

#endif   // DFM_ACTIONTYPE_DEFINES_H
