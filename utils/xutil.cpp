// Copyright (c) 2015 Deepin Ltd. All rights reserved.
// Use of this source is governed by Commercial License that can be found
// in the LICENSE file.
#include "xutil.h"

#include <QDebug>
#include <QMouseEvent>
#include <QWidget>
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

/* From the WM spec */
#define _NET_WM_MOVERESIZE_INVALID          -1
#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
#define _NET_WM_MOVERESIZE_SIZE_TOP          1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

namespace QtX11 {

namespace {

const char kAtomNameHidden[] = "_NET_WM_STATE_HIDDEN";
const char kAtomNameMaximizedHorz[] = "_NET_WM_STATE_MAXIMIZED_HORZ";
const char kAtomNameMaximizedVert[] = "_NET_WM_STATE_MAXIMIZED_VERT";
const char kAtomNameMoveResize[] = "_NET_WM_MOVERESIZE";
const char kAtomNameWmState[] = "_NET_WM_STATE";

enum class CornerEdge {
  kInvalid = 0,
  kTop = 1,
  kRight = 2,
  kBottom = 4,
  kLeft = 8,
  kTopLeft = 1 | 8,
  kTopRight = 1 | 2,
  kBottomLeft = 4 | 8,
  kBottomRight = 4 | 2,
};

enum class XCursorType { //deatil in /usr/include/cursorfont.h
  kHandle1 = 58,//handle1
  kHandle2 = 8, // handle2
  kInvalid = 2, // arrow cursor.
  kTop = 138,
  kTopRight = 136,
  kRight = 96,
  kBottomRight = 14,
  kBottom = 16,
  kBottomLeft = 12,
  kLeft = 70,
  kTopLeft = 134,
};

int CornerEdge2WmGravity(const CornerEdge& ce) {
  switch (ce) {
  case CornerEdge::kTopLeft:     return _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
  case CornerEdge::kTop:         return _NET_WM_MOVERESIZE_SIZE_TOP;
  case CornerEdge::kTopRight:    return _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
  case CornerEdge::kRight:       return _NET_WM_MOVERESIZE_SIZE_RIGHT;
  case CornerEdge::kBottomRight: return _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
  case CornerEdge::kBottom:      return _NET_WM_MOVERESIZE_SIZE_BOTTOM;
  case CornerEdge::kBottomLeft:  return _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
  case CornerEdge::kLeft:        return _NET_WM_MOVERESIZE_SIZE_LEFT;
  default:                       return _NET_WM_MOVERESIZE_INVALID;
  }
}

XCursorType CornerEdge2XCursor(const CornerEdge& ce) {
  switch (ce) {
  case CornerEdge::kTop:         return XCursorType::kTop;
  case CornerEdge::kTopRight:    return XCursorType::kTopRight;
  case CornerEdge::kRight:       return XCursorType::kRight;
  case CornerEdge::kBottomRight: return XCursorType::kBottomRight;
  case CornerEdge::kBottom:      return XCursorType::kBottom;
  case CornerEdge::kBottomLeft:  return XCursorType::kBottomLeft;
  case CornerEdge::kLeft:        return XCursorType::kLeft;
  case CornerEdge::kTopLeft:     return XCursorType::kTopLeft;
  default:                       return XCursorType::kInvalid;
  }
}

void ChangeWindowMaximizedState(QWidget* widget, int wm_state) {
  const auto display = QX11Info::display();
  const auto screen = QX11Info::appScreen();

  XEvent xev;
  memset(&xev, 0, sizeof(xev));
  const Atom net_wm_state = XInternAtom(display, kAtomNameWmState, false);
  const Atom vertical_maximized = XInternAtom(display,
                                              kAtomNameMaximizedVert,
                                              false);
  const Atom horizontal_maximized = XInternAtom(display,
                                                kAtomNameMaximizedHorz,
                                                false);

  xev.xclient.type = ClientMessage;
  xev.xclient.message_type = net_wm_state;
  xev.xclient.display = display;
  xev.xclient.window = widget->winId();
  xev.xclient.format = 32;

  xev.xclient.data.l[0] = wm_state;
  xev.xclient.data.l[1] = vertical_maximized;
  xev.xclient.data.l[2] = horizontal_maximized;
  xev.xclient.data.l[3] = 1;

  XSendEvent(display,
             QX11Info::appRootWindow(screen),
             false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &xev);
}

CornerEdge GetCornerEdge(QWidget* widget, int x, int y,
                         int border_inner_size) {
  const int window_width = widget->width();
  const int window_height = widget->height();
  unsigned int ce = static_cast<unsigned int>(CornerEdge::kInvalid);

  if (0 <= y && y < border_inner_size) {
    ce = ce | static_cast<unsigned int>(CornerEdge::kTop);
  }
  if (0 <= x && x < border_inner_size) {
    ce = ce | static_cast<unsigned int>(CornerEdge::kLeft);
  }
  if (window_height - border_inner_size < y &&
      y < window_height) {
    ce = ce | static_cast<unsigned int>(CornerEdge::kBottom);
  }
  if (window_width - border_inner_size < x &&
      x < window_width) {
    ce = ce | static_cast<unsigned int>(CornerEdge::kRight);
  }
  return static_cast<CornerEdge>(ce);
}

void SendMoveResizeMessage(QWidget* widget, int action, int button) {
  const auto display = QX11Info::display();
  const auto screen = QX11Info::appScreen();

  XEvent xev;
  memset(&xev, 0, sizeof(xev));
  const Atom net_move_resize = XInternAtom(display, kAtomNameMoveResize, false);
  xev.xclient.type = ClientMessage;
  xev.xclient.message_type = net_move_resize;
  xev.xclient.display = display;
  xev.xclient.window = widget->winId();
  xev.xclient.format = 32;

  const auto global_position = QCursor::pos();
  xev.xclient.data.l[0] = global_position.x();
  xev.xclient.data.l[1] = global_position.y();
  xev.xclient.data.l[2] = action;
  xev.xclient.data.l[3] = button;
  xev.xclient.data.l[4] = 1;  // source indication
  XUngrabPointer(display, QX11Info::appTime());

  XSendEvent(display,
             QX11Info::appRootWindow(screen),
             false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &xev);
}

}  // namespace

namespace utils {

void MoveWindow(QWidget *widget, QMouseEvent *event, const QRect &dragableRect)
{
    if (dragableRect.contains(event->pos())){
        SendMoveResizeMessage(widget, _NET_WM_MOVERESIZE_MOVE, Button1);
    }
}

void ResizeWindow(QWidget *widget, QMouseEvent *event, int border_inner_size)
{
    const CornerEdge ce = GetCornerEdge(widget, event->x(), event->y(),
                                        border_inner_size);
    if (ce != CornerEdge::kInvalid) {
      const int action = CornerEdge2WmGravity(ce);
      SendMoveResizeMessage(widget, action, Button1);
    }
}


void ShowMaximizedWindow(QWidget* widget) {
  ChangeWindowMaximizedState(widget, _NET_WM_STATE_ADD);
}

void ShowMinimizedWindow(QWidget* widget) {
  const auto display = QX11Info::display();
  const auto screen = QX11Info::appScreen();

  XEvent xev;
  memset(&xev, 0, sizeof(xev));
  const Atom net_wm_state = XInternAtom(display, kAtomNameWmState, false);
  const Atom hidden = XInternAtom(display, kAtomNameHidden, false);
  xev.xclient.type = ClientMessage;
  xev.xclient.message_type = net_wm_state;
  xev.xclient.display = display;
  xev.xclient.window = widget->winId();
  xev.xclient.format = 32;

  xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
  xev.xclient.data.l[1] = hidden;
  xev.xclient.data.l[2] = 0;
  xev.xclient.data.l[3] = 1;

  XSendEvent(display,
             QX11Info::appRootWindow(screen),
             false,
             SubstructureRedirectMask | SubstructureNotifyMask,
             &xev
  );
  XIconifyWindow(display, widget->winId(), screen);
}

void ShowNormalWindow(QWidget* widget) {
  ChangeWindowMaximizedState(widget, _NET_WM_STATE_REMOVE);
}

void ToggleMaximizedWindow(QWidget* widget) {
  ChangeWindowMaximizedState(widget, _NET_WM_STATE_TOGGLE);
}

bool UpdateCursorShape(QWidget* widget, int x, int y, int border_inner_size) {
    const CornerEdge ce = GetCornerEdge(widget, x, y, border_inner_size);
    const auto display = QX11Info::display();
    const WId window_id = widget->winId();

    const XCursorType x_cursor = CornerEdge2XCursor(ce);
    const Cursor cursor = XCreateFontCursor(display,
    static_cast<unsigned int>(x_cursor));
    if (!cursor) {
      return false;
        qWarning() << "[ui]::UpdateCursorShape() call XCreateFontCursor() "
                    << "failed!";
    }
    const int result = XDefineCursor(display, window_id, cursor);
    if (result){
        if (ce != CornerEdge::kInvalid){
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool setNormalCursor(QWidget* widget)
{
    const XCursorType x_cursor(XCursorType::kInvalid);
    unsigned int cursorType = static_cast<unsigned int>(x_cursor);
    return setCursorByCursorType(widget, cursorType);
}

bool setHandleCursor(QWidget *widget)
{
    const XCursorType x_cursor(XCursorType::kHandle2);
    unsigned int cursorType = static_cast<unsigned int>(x_cursor);
    return setCursorByCursorType(widget, cursorType);
}

bool setCursorByCursorType(QWidget *widget, unsigned int cursorType)
{
    const auto display = QX11Info::display();
    const WId window_id = widget->winId();

    const Cursor cursor = XCreateFontCursor(display, cursorType);
    if (!cursor) {
      return false;
        qWarning() << "[ui]::UpdateCursorShape() call XCreateFontCursor() "
                    << "failed!";
    }
    const int result = XDefineCursor(display, window_id, cursor);
    if (result){
        return true;
    }else{
        return false;
    }
}



}  // namespace utils
}  // namespace ui
