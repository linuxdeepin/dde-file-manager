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
inline constexpr char kNewFolder[] = "new-folder";
inline constexpr char kNewDoc[] = "new-document";
inline constexpr char kNewOfficeText[] = "new-office-text";
inline constexpr char kNewSpreadsheets[] = "new-spreadsheets";
inline constexpr char kNewPresentation[] = "new-presentation";
inline constexpr char kNewPlainText[] = "new-plain-text";

// ClipBoardMenu
inline constexpr char kPaste[] = "paste";
inline constexpr char kCut[] = "cut";
inline constexpr char kCopy[] = "copy";

// OpenDirMenu
inline constexpr char kOpenAsAdmin[] = "open-as-administrator";
inline constexpr char kSelectAll[] = "select-all";
inline constexpr char kOpenInNewWindow[] = "open-in-new-window";
inline constexpr char kOpenInNewTab[] = "open-in-new-tab";
inline constexpr char kOpenInTerminal[] = "open-in-terminal";

// FileOperatorMenu
inline constexpr char kOpen[] = "open";
inline constexpr char kRename[] = "rename";
inline constexpr char kDelete[] = "delete";
inline constexpr char kCreateSymlink[] = "create-system-link";
inline constexpr char kEmptyTrash[] = "empty-trash";
inline constexpr char kSetAsWallpaper[] = "set-as-wallpaper";

// OpenWithMenu
inline constexpr char kOpenWith[] = "open-with";
inline constexpr char kOpenWithCustom[] = "open-with-custom";
inline constexpr char kOpenWithApp[] = "open-with-app";

// DirShareMenu
inline constexpr char kDirShare[] = "dir-share";

// ShareMenu
inline constexpr char kShare[] = "share";
inline constexpr char kShareToBluetooth[] = "share-to-bluetooth";

// SendToMenu
inline constexpr char kSendTo[] = "send-to";
inline constexpr char kSendToRemovablePrefix[] = "send-to-removable-";
inline constexpr char kSendToDesktop[] = "send-to-desktop";

// Separator
inline constexpr char kSeparator[] = "separator-line";
}
}

#endif   // ACTION_DEFINES_H
