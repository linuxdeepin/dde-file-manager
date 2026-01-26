// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTFILTERUTILS_H
#define EVENTFILTERUTILS_H

#include <dfm-base/dfm_base_global.h>

#include <QEvent>

namespace dfmbase {
namespace EventFilter {

/**
 * @brief Handle right-click outside menu (only effective in X11 environment)
 * @param event The event to process
 * @param context Context object (optional, for QObject context)
 * @return true Event has been processed and blocked, false continue propagation
 */
bool handleRightClickOutsideMenu(QEvent *event, QObject *context = nullptr);

}   // namespace EventFilter
}   // namespace dfmbase

#endif   // EVENTFILTERUTILS_H
