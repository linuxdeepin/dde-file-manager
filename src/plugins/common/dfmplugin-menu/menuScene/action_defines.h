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
#ifndef ACTION_DEFINES_H
#define ACTION_DEFINES_H

namespace dfmplugin_menu {
namespace ActionID {
// NewCreateMenu
static constexpr char kNewFolder[] = "new-folder";
static constexpr char kNewDoc[] = "new-document";
static constexpr char kNewOfficeText[] = "new-office-text";
static constexpr char kNewSpreadsheets[] = "new-spreadsheets";
static constexpr char kNewPresentation[] = "new-presentation";
static constexpr char kNewPlainText[] = "new-plain-text";

// ClipBoardMenu
static constexpr char kPaste[] = "paste";
static constexpr char kCut[] = "cut";
static constexpr char kCopy[] = "copy";

// OpenDirMenu
static constexpr char kOpenAsAdmin[] = "open-as-administrator";
static constexpr char kSelectAll[] = "select-all";
static constexpr char kOpenInNewWindow[] = "open-in-new-window";
static constexpr char kOpenInNewTab[] = "open-in-new-tab";
static constexpr char kOpenInTerminal[] = "open-in-terminal";

// FileOperatorMenu
static constexpr char kOpen[] = "open";
static constexpr char kRename[] = "rename";
static constexpr char kDelete[] = "delete";

// "OpenWithMenu"
static constexpr char kOpenWith[] = "open-with";

// SendToMenu
static constexpr char kSendTo[] = "send-to";
static constexpr char kSendToBluetooth[] = "send-to-bluetooth";
static constexpr char kSendToRemovablePrefix[] = "send-to-removable-";
}
}

#endif   // ACTION_DEFINES_H
