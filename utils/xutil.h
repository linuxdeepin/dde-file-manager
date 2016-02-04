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
