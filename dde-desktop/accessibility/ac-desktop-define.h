/*
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * Author:     max-lv <lvwujun@uniontech.com>
 *
 * Maintainer: max-lv <lvwujun@uniontech.com>
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

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>
#include "accessibility/acintelfunctions.h"

// 使用宏定义，方便国际化操作

#ifdef ENABLE_ACCESSIBILITY
    #define AC_SCREEN_BACKGROUND QObject::tr("screen_background") // 设置桌面
    #define AC_CANVAS_GRID_VIEW QObject::tr("screen_canvas_view") // 桌面视图画布
    #define AC_WATER_MASK_FRAME QObject::tr("water_mask") // 桌面水印

    #define AC_WATER_MASK_LOGO_LABEL QObject::tr("water_mask_logo") // 水印logo
    #define AC_WATER_MASK_TEXT QObject::tr("water_mask_text") // 水印文字描述
    #define AC_WATER_TEXT_LABEL_NO_AUTHORIZED QObject::tr("Not_authorized") // 水印文字描述:无授权
    #define AC_WATER_TEXT_LABEL_IN_TRIAL QObject::tr("In_trial_period") // 水印文字描述:试用

    #define AC_FILE_MENU_DESKTOP QObject::tr("desktop_menu")
#else
    #define AC_SCREEN_BACKGROUND "" // 设置桌面
    #define AC_CANVAS_GRID_VIEW "" // 桌面视图画布
    #define AC_WATER_MASK_FRAME "" // 桌面水印

    #define AC_WATER_MASK_LOGO_LABEL "" // 水印logo
    #define AC_WATER_MASK_TEXT "" // 水印文字描述
    #define AC_WATER_TEXT_LABEL_NO_AUTHORIZED "" // 水印文字描述:无授权
    #define AC_WATER_TEXT_LABEL_IN_TRIAL "" // 水印文字描述:试用

    #define AC_FILE_MENU_DESKTOP ""
#endif

#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
