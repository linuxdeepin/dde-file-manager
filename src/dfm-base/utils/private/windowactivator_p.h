// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWACTIVATOR_P_H
#define WINDOWACTIVATOR_P_H

#include <dfm-base/dfm_base_global.h>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

namespace windowactivator {

/*!
 * \brief Starts listening to the compositor's toplevels
 *
 * Binding is asynchronous and the compositor only describes the toplevels that
 * already existed when the bind reached it, so the manager has to be created
 * before the windows it will later act on. Called from
 * FileManagerWindowsManager::createWindow(), where every file manager window is
 * born, which leaves the event loop enough turns to learn about them long
 * before one of them can be minimized.
 */
void bindToplevelManager();

/*!
 * \brief Asks the compositor to restore \a window and give it focus
 *
 * On Wayland a client cannot lift its own minimized state: xdg-shell exposes
 * set_minimized() alone, and Qt never reports Qt::WindowMinimized, so
 * QWidget::isMinimized() is always false and showNormal() sends nothing.
 * Treeland does offer a client side way to ask, through
 * treeland_foreign_toplevel_manager_v2. The manager only exists on Wayland, so
 * there is nothing to restore on the other platforms.
 *
 * Returns \c false when there is nothing to ask - the compositor does not
 * expose that protocol, or it reports no toplevel for \a window - so the caller
 * can fall back to xdg-activation.
 */
bool restoreAndActivate(QWindow *window);

}   // namespace windowactivator

DFMBASE_END_NAMESPACE

#endif   // WINDOWACTIVATOR_P_H
