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

namespace DesktopUtil
{
    //根据主题加载系统中的x11光标为QCursor
    QCursor *loadQCursorFromX11Cursor(const char* theme, const char* cursorName, int cursorSize);
}
