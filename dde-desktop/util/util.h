/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once
#include <QtCore>

class QWidget;
namespace DesktopUtil
{
    //根据主题加载系统中的x11光标为QCursor，桌面走wayland不需要从x11获取光标
    //QCursor *loadQCursorFromX11Cursor(const char* theme, const char* cursorName, int cursorSize);
    void set_desktop_window(QWidget *);
    void set_prview_window(QWidget *w);
}
