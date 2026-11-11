// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDESHELLWINDOW_P_H
#define DDESHELLWINDOW_P_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

namespace ddeshell {

/*!
 * \brief Hides \a window from the Treeland dock taskbar
 *
 * Client side of treeland-dde-shell-v1: the window's wl_surface gets a shell
 * surface whose skip-dock-preview bit is set, which makes the compositor keep
 * the window out of the foreign toplevel lists the dock taskbar consumes.
 *
 * The manager binds asynchronously and the platform surface does not exist
 * before the window is first shown, so a call made before either is ready is
 * retried automatically once it arrives. Only one shell surface may be
 * associated with a wl_surface, so it is created once per surface and follows
 * the window's surface across re-creations.
 *
 * Returns false when there is nothing to hide with (no Wayland platform), in
 * which case the window keeps its default dock entry. On Wayland the result
 * only means the request was accepted; the compositor may still not expose
 * the protocol, and the window then keeps its dock entry as well.
 */
bool setSkipDockPreview(QWindow *window, bool skip);

}   // namespace ddeshell

DFMBASE_END_NAMESPACE

#endif   // DDESHELLWINDOW_P_H
