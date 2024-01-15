// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEMENU_DEFINES_H
#define WORKSPACEMENU_DEFINES_H

namespace dfmplugin_workspace {

namespace ActionID {
inline constexpr char kSortBy[] = "sort-by";
inline constexpr char kDisplayAs[] = "display-as";
inline constexpr char kRefresh[] = "refresh";

// sort by
inline constexpr char kSrtName[] = "sort-by-name";
inline constexpr char kSrtTimeModified[] = "sort-by-time-modified";
inline constexpr char kSrtSize[] = "sort-by-size";
inline constexpr char kSrtType[] = "sort-by-type";

// display by
inline constexpr char kDisplayIcon[] = "display-as-icon";
inline constexpr char kDisplayList[] = "display-as-list";
inline constexpr char kDisplayTree[] = "display-as-tree";
}

}

#endif   // WORKSPACEMENU_DEFINES_H
