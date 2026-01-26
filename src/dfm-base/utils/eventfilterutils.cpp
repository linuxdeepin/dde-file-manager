// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventfilterutils.h"
#include "windowutils.h"

#include <QMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QTimer>
#include <QContextMenuEvent>

namespace dfmbase {
namespace EventFilter {

bool handleRightClickOutsideMenu(QEvent *event, QObject *context)
{
    // Only effective in X11 environment
    if (!WindowUtils::isX11()) {
        return false;
    }

    if (event->type() != QEvent::MouseButtonPress) {
        return false;
    }

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() != Qt::RightButton) {
        return false;
    }

    QWidget *activePopup = QApplication::activePopupWidget();
    QMenu *activeMenu = qobject_cast<QMenu *>(activePopup);
    if (!activeMenu) {
        return false;
    }

    QPoint globalPos = mouseEvent->globalPosition().toPoint();

    // Check if click is inside menu or its visible submenus
    bool insideMenu = activeMenu->geometry().contains(globalPos);
    if (!insideMenu) {
        QAction *active = activeMenu->activeAction();
        if (active && active->menu() && active->menu()->isVisible()) {
            insideMenu = active->menu()->geometry().contains(globalPos);
        }
    }

    if (insideMenu) {
        return false;
    }

    // Click outside menu: close the menu first
    activeMenu->close();

    // Delay triggering new context menu
    QTimer::singleShot(0, context ? context : qApp, [globalPos]() {
        QWidget *targetWidget = QApplication::widgetAt(globalPos);
        if (targetWidget) {
            QPoint localPos = targetWidget->mapFromGlobal(globalPos);
            QContextMenuEvent *contextEvent = new QContextMenuEvent(
                    QContextMenuEvent::Mouse,
                    localPos,
                    globalPos);
            QApplication::postEvent(targetWidget, contextEvent);
        }
    });

    return true;   // Block event
}

}   // namespace EventFilter
}   // namespace dfmbase
