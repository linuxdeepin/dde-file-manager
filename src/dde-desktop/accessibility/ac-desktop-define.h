/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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


    #define AC_SCREEN_BACKGROUND                    "screen_background" // 设置桌面
    #define AC_CANVAS_GRID_VIEW                     "screen_canvas_view" // 桌面视图画布
    #define AC_WATER_MASK_FRAME                     "water_mask" // 桌面水印

    #define AC_WATER_MASK_LOGO_LABEL                "water_mask_logo" // 水印logo
    #define AC_WATER_MASK_TEXT                      "water_mask_text" // 水印文字描述
    #define AC_WATER_TEXT_LABEL_NO_AUTHORIZED       "Not_authorized" // 水印文字描述:无授权
    #define AC_WATER_TEXT_LABEL_IN_TRIAL            "In_trial_period" // 水印文字描述:试用

    #define AC_FILE_MENU_DESKTOP                    "desktop_menu"


#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
