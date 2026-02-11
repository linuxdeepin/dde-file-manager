// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
inline constexpr char kReverseSelect[] = "reverse-select";

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

// SendToMenu
inline constexpr char kSendTo[] = "send-to";
inline constexpr char kSendToRemovablePrefix[] = "send-to-removable-";
inline constexpr char kSendToDesktop[] = "send-to-desktop";
inline constexpr char kSendToBluetooth[] = "send-to-bluetooth";

// Separator
inline constexpr char kSeparator[] = "separator-line";

// ViewMenu
inline constexpr char kDisplayAs[] = "display-as";
inline constexpr char kSortBy[] = "sort-by";
inline constexpr char kGroupBy[] = "group-by";
}
}

#endif   // ACTION_DEFINES_H
