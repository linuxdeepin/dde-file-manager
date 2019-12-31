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
