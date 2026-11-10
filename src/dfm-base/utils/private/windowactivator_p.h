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
 * \brief Starts tracking the compositor's toplevels
 *
 * Binding the manager is asynchronous and the compositor only describes the
 * toplevels that already existed when the bind reached it, so the manager has
 * to be created before the windows it will later act on. Called where the
 * application starts opening windows, which leaves the event loop enough turns
 * to learn about them long before one of them can be minimized.
 */
void prepare();

/*!
 * \brief Asks the compositor to restore \a window and give it focus
 *
 * xdg-shell exposes set_minimized() alone, so a client cannot lift its own
 * minimized state, and xdg-activation is of no use here: the compositor only
 * trusts a token minted by a window the user is interacting with, which a
 * minimized window never is. zwlr_foreign_toplevel_manager_v1 is what window
 * switchers use instead, and it lets a client act on its own toplevels.
 *
 * Returns \c false when there is nothing to ask - the platform is not Wayland,
 * the compositor does not expose the protocol, or it reports no toplevel for
 * \a window - so the caller can fall back to a plain activation request.
 */
bool restoreAndActivate(QWindow *window);

}   // namespace windowactivator

DFMBASE_END_NAMESPACE

#endif   // WINDOWACTIVATOR_P_H
