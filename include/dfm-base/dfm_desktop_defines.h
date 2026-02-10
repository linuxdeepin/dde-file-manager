// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_DESKTOP_DEFINES_H
#define DFM_DESKTOP_DEFINES_H

#include <dfm-base/dfm_base_global.h>

namespace dfmbase {

//显示模式
enum DisplayMode {
    kCustom = 0,
    kDuplicate,
    kExtend,
    kShowonly
};

namespace DesktopFrameProperty {
// property for window
inline constexpr char kPropScreenName[] = "ScreenName";
inline constexpr char kPropIsPrimary[] = "IsPrimary";
inline constexpr char kPropScreenGeometry[] = "ScreenGeometry";
inline constexpr char kPropScreenAvailableGeometry[] = "ScreenAvailableGeometry";
inline constexpr char kPropScreenHandleGeometry[] = "ScreenHandleGeometry";

// property for window and all sub widgets
inline constexpr char kPropWidgetName[] = "WidgetName";
inline constexpr char kPropWidgetLevel[] = "WidgetLevel";
}

}

#endif // DFM_DESKTOP_DEFINES_H
