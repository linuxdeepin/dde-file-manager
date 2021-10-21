/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "autoactivatewindow.h"
#include "autoactivatewindow_p.h"
#include "util/dde/desktopinfo.h"

#include <QWidget>
#include <QWindow>
#include <QTimer>
#include <QDebug>

namespace {
static xcb_screen_t *screen_of_display(xcb_connection_t *c, int screen)
{
    xcb_screen_iterator_t iter;
    iter = xcb_setup_roots_iterator(xcb_get_setup(c));
    for ( ; iter.rem; --screen, xcb_screen_next(&iter))
     if (screen == 0)
       return iter.data;

    return NULL;
}

xcb_window_t getRootWindow(xcb_connection_t *c, xcb_window_t cur)
{
    xcb_window_t window = cur;
    xcb_query_tree_cookie_t cookie = xcb_query_tree(c, cur);
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(c, cookie, nullptr);
    if (!reply)
        return 0;

    if (window != reply->root && reply->parent != reply->root) {
        window = reply->parent;
        window = getRootWindow(c, window);
    }

    free(reply);
    return window;
}

}
AutoActivateWindowPrivate::AutoActivateWindowPrivate(AutoActivateWindow *parent)
    : QObject(parent)
    , q(parent)
{

}

AutoActivateWindowPrivate::~AutoActivateWindowPrivate()
{
    if (m_x11Con) {
        xcb_disconnect(m_x11Con);
        m_x11Con = nullptr;
    }
}

void AutoActivateWindowPrivate::watchOnWayland(bool on)
{
    Q_ASSERT(m_watchedWidget);
    QWindow *window = m_watchedWidget->windowHandle();
    Q_ASSERT(window);

    if (on) {
        q->connect(window, &QWindow::activeChanged, this, [ = ]() {
            if (m_watchedWidget == nullptr || m_watchedWidget->isActiveWindow())
                return;
            //激活窗口
            m_watchedWidget->activateWindow();
            //10毫秒后再次检测
            QTimer::singleShot(10, m_watchedWidget, [ = ]() {
                if (m_watchedWidget && !m_watchedWidget->isActiveWindow())
                    m_watchedWidget->windowHandle()->activeChanged();
            });
        });
    } else {
        q->disconnect(window, &QWindow::activeChanged, this, nullptr);
    }
}

void AutoActivateWindowPrivate::watchOnX11(bool on)
{
    Q_ASSERT(m_watchedWidget);
    QWindow *window = m_watchedWidget->windowHandle();
    Q_ASSERT(window);

    if (on) {
        if (initConnect()) {
            q->connect(window, &QWindow::activeChanged, this, [=]() {
                if (!m_watchedWidget)
                    return;

                if (m_watchedWidget->isActiveWindow())
                    m_checkTimer.stop();
                else {
                    //300ms检测一次
                    m_checkTimer.setInterval(300);
                    m_checkTimer.start();
                    QMetaObject::invokeMethod(this, "checkWindowOnX11", Qt::QueuedConnection);
                }
            });
            m_watchedWin = getRootWindow(m_x11Con, m_watchedWidget->winId());
        }
    } else {
        q->disconnect(window, &QWindow::activeChanged, this, nullptr);
        m_checkTimer.stop();
    }
}

void AutoActivateWindowPrivate::checkWindowOnX11()
{
    if (!m_watchedWidget || !m_x11Con)
        return;

    if (m_watchedWidget->isActiveWindow()) {
        return;
    }

    xcb_query_tree_cookie_t cookie = xcb_query_tree(m_x11Con, m_rootWin);
    xcb_generic_error_t *err;
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(m_x11Con, cookie, &err);
    if (!reply) {
        qWarning() << "can not get reply: xcb_query_tree";
        return;
    }

    if (err) {
        qWarning() << "xcb_query_tree failed with error code " << err->error_code;
        free(reply);
        return;
    }

    xcb_window_t *children = xcb_query_tree_children(reply);
    int count = xcb_query_tree_children_length(reply);
    for (int i = count - 1;  i > -1; i--) {
        xcb_get_window_attributes_reply_t *attrreply = xcb_get_window_attributes_reply(
                    m_x11Con, xcb_get_window_attributes(m_x11Con, children[i]), nullptr);
        bool viewable = attrreply->map_state == XCB_MAP_STATE_VIEWABLE;
        free(attrreply);

        if (children[i] == m_watchedWin) {
            // 使用xcb_set_input_focus，呼出启动器会出现不拿走焦点的问题
            //xcb_set_input_focus(m_x11Con, XCB_NONE, m_watchedWidget->winId(), XCB_CURRENT_TIME);
            m_watchedWidget->activateWindow();
            break;
        }

        if (viewable)
            break;
        else
            continue;
    }

    free(reply);
}

bool AutoActivateWindowPrivate::initConnect()
{
    if (!m_x11Con) {
        int nbr;
        m_x11Con = xcb_connect(nullptr, &nbr);
        Q_ASSERT(m_x11Con);

        if (nbr < 0) {
            qWarning() << "xcb_connect fail." << nbr;
            xcb_disconnect(m_x11Con);
            m_x11Con = nullptr;
            return false;
        }

        xcb_screen_t *screen = screen_of_display(m_x11Con, nbr);
        Q_ASSERT(screen);
        m_rootWin = screen->root;

        connect(&m_checkTimer, &QTimer::timeout, this, &AutoActivateWindowPrivate::checkWindowOnX11);
    }

    return true;
}

AutoActivateWindow::AutoActivateWindow(QObject *parent)
    : QObject(parent)
    , d(new AutoActivateWindowPrivate(this))
{

}

void AutoActivateWindow::setWatched(QWidget *win)
{
    if (d->m_run)
        return;

    d->m_watchedWidget = win;
}

bool AutoActivateWindow::start()
{
    if (d->m_run || !d->m_watchedWidget)
        return false;

    //监控窗口状态
    if (DesktopInfo().waylandDectected()) {
        d->watchOnWayland(true);
    } else {
        d->watchOnX11(true);
    }

    d->m_run = true;
    return true;
}

void AutoActivateWindow::stop()
{
    d->m_run = false;
    if (DesktopInfo().waylandDectected()) {
        d->watchOnWayland(false);
    } else {
        d->watchOnX11(false);
    }
}
