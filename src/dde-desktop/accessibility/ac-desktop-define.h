// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
