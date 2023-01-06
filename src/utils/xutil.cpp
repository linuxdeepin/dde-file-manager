// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xutil.h"

#include <QDebug>
#include <QWidget>
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */

namespace QtX11 {

namespace {

const char kAtomNameMaximizedHorz[] = "_NET_WM_STATE_MAXIMIZED_HORZ";
const char kAtomNameMaximizedVert[] = "_NET_WM_STATE_MAXIMIZED_VERT";
const char kAtomNameWmState[] = "_NET_WM_STATE";

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

}  // namespace

namespace utils {

void ShowNormalWindow(QWidget* widget) {
  ChangeWindowMaximizedState(widget, _NET_WM_STATE_REMOVE);
}

}  // namespace utils
}  // namespace ui
