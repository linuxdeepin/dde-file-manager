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

#define SCREEN_BACKGROUND QObject::tr("screen_background")
#define OBJ_NAME_CANVAS_GRID_VIEW QObject::tr("screen_frame")
#define OBJ_NAME_WATER_MASK_FRAME QObject::tr("water_mask_frame")

#define WATER_MASK_LOGO_LABEL QObject::tr("water_mask_logo")
#define WATER_MASK_TEXT_LABEL QObject::tr("water_mask_text")

#endif // DESKTOP_FRAME_ACCESSIBLE_DEFINE_H
