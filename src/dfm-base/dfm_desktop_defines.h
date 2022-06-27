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
#ifndef DFM_DESKTOP_DEFINES_H
#define DFM_DESKTOP_DEFINES_H

#include "dfm-base/dfm_base_global.h"

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
