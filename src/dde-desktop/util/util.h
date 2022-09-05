// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
