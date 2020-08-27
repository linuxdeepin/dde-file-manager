/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * This is a static file that used to define the UI object name that will used to auto-test
 **/

#ifndef DESKTOP_FRAME_ACCESSIBLE_DEFINE_H
#define DESKTOP_FRAME_ACCESSIBLE_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作

#define SCREEN_BACKGROUND QObject::tr("screen_background") // 设置桌面
#define OBJ_NAME_CANVAS_GRID_VIEW QObject::tr("screen_frame") // 桌面视图画布
#define OBJ_NAME_WATER_MASK_FRAME QObject::tr("water_mask_frame") // 桌面水印

#define WATER_MASK_LOGO_LABEL QObject::tr("water_mask_logo") // 水印logo
#define WATER_MASK_TEXT_LABEL QObject::tr("water_mask_text") // 水印文字描述

#endif // DESKTOP_FRAME_ACCESSIBLE_DEFINE_H
