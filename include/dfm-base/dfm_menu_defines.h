// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_MENU_DEFINES_H
#define DFM_MENU_DEFINES_H

#include <dfm-base/dfm_base_global.h>

DFMBASE_BEGIN_NAMESPACE

namespace MenuParamKey {
// file menu params for initialize
inline constexpr char kCurrentDir[] = "currentDir";   // QUrl
inline constexpr char kSelectFiles[] = "selectFiles";   // QList<QUrl>
inline constexpr char kTreeSelectFiles[] = "treeSelectFiles";   // QList<QUrl>
inline constexpr char kOnDesktop[] = "onDesktop";   // bool
inline constexpr char kWindowId[] = "windowId";   // quint64
inline constexpr char kIsEmptyArea[] = "isEmptyArea";   // bool
inline constexpr char kIndexFlags[] = "indexFlags";   // Qt::ItemFlags
inline constexpr char kIsSystemPathIncluded[] = "isSystemPathIncluded";   // bool, true if 'SystemPathUtil::isSystemPath' return true
inline constexpr char kIsDDEDesktopFileIncluded[] = "isDDEDesktopFileIncluded";   // bool, contains 'dde-computer.desktop','dde-trash.desktop' and 'dde-home.desktop'
inline constexpr char kIsFocusOnDDEDesktopFile[] = "isFocusOnDDEDesktopFile";   // bool
}

namespace ActionPropertyKey {
// key for action property
inline constexpr char kActionID[] = "actionID";   // string

}

DFMBASE_END_NAMESPACE

#endif   // DFM_MENU_DEFINES_H
