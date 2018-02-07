/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

// Copyright (c) 2015 Deepin Ltd. All rights reserved.
// Use of this source is governed by Commercial License that can be found
// in the LICENSE file.
#ifndef UI_UTILS_XUTIL_H_
#define UI_UTILS_XUTIL_H_

class QMouseEvent;
class QWidget;
class QRect;
class QPoint;

namespace  QtX11{
namespace utils {

namespace {

// Default inner border size.
//const int kBorderInnerSize = 6;

}  // namespace

// Move window to cursor position.
void MoveWindow(QWidget* widget, QMouseEvent* event,
                const QRect& dragableRect);

// Move or resize window, based on mouse event.
// |border_inner_size| is treated as border width. Cursor shapes changes in
// border area.

void ResizeWindow(QWidget* widget, QMouseEvent* event,
                      int border_inner_size = 6);

// Shows the widget maximized.
// Note that |widget| shall be a window.
void ShowMaximizedWindow(QWidget* widget);

// Show the widget minimized.
void ShowMinimizedWindow(QWidget* widget);

// Restores the widget after it has been maximized or minimized.
void ShowNormalWindow(QWidget* widget);

// Toggle window state between normal size and maximized size.
void ToggleMaximizedWindow(QWidget* widget);

// Update cursor shape when cursor reaches cursor or edge of widget border.
// Returns false if failed to change cursor shape.

bool setNormalCursor(QWidget* widget);
bool setHandleCursor(QWidget* widget);

bool setCursorByCursorType(QWidget* widget, unsigned int cursorType);

bool UpdateCursorShape(QWidget* widget, int x, int y, int border_inner_size = 6);

}  // namespace utils
}  // namespace ui

#endif  // UI_UTILS_XUTIL_H_
