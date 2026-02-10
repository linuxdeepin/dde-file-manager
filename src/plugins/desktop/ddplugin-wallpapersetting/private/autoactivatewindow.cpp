// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "autoactivatewindow.h"
#include "autoactivatewindow_p.h"

#include <dfm-base/utils/windowutils.h>

#include <QWidget>
#include <QWindow>
#include <QTimer>
#include <QDebug>

using namespace ddplugin_wallpapersetting;
DFMBASE_USE_NAMESPACE

namespace {
static xcb_screen_t *screen_of_display(xcb_connection_t *c, int screen)
{
    xcb_screen_iterator_t iter;
    iter = xcb_setup_roots_iterator(xcb_get_setup(c));
    for (; iter.rem; --screen, xcb_screen_next(&iter))
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
    : QObject(parent), q(parent)
{
}

AutoActivateWindowPrivate::~AutoActivateWindowPrivate()
{
    if (x11Con) {
        xcb_disconnect(x11Con);
        x11Con = nullptr;
    }
}

void AutoActivateWindowPrivate::watchOnWayland(bool on)
{
    Q_ASSERT(watchedWidget);
    if (watchedWidget == nullptr) {
        fmWarning() << "Cannot watch on Wayland: watchedWidget is null";
        return;
    }

    QWindow *window = watchedWidget->windowHandle();
    Q_ASSERT(window);
    if (window == nullptr) {
        fmWarning() << "Cannot watch on Wayland: window handle is null";
        return;
    }

    if (on) {
        q->connect(window, &QWindow::activeChanged, this, [=]() {
            if (watchedWidget == nullptr || watchedWidget->isActiveWindow())
                return;
            //激活窗口
            watchedWidget->activateWindow();
            //10毫秒后再次检测
            QTimer::singleShot(10, watchedWidget, [=]() {
                if (watchedWidget && !watchedWidget->isActiveWindow())
                    watchedWidget->windowHandle()->activeChanged();
            });
        });
    } else {
        q->disconnect(window, &QWindow::activeChanged, this, nullptr);
    }
}

void AutoActivateWindowPrivate::watchOnX11(bool on)
{
    Q_ASSERT(watchedWidget);
    if (watchedWidget == nullptr) {
        fmWarning() << "Cannot watch on X11: watchedWidget is null";
        return;
    }

    QWindow *window = watchedWidget->windowHandle();
    Q_ASSERT(window);
    if (window == nullptr) {
        fmWarning() << "Cannot watch on X11: window handle is null";
        return;
    }

    if (on) {
        if (initConnect()) {
            fmInfo() << "Starting X11 window activation monitoring";
            q->connect(window, &QWindow::activeChanged, this, [=]() {
                if (!watchedWidget)
                    return;

                if (watchedWidget->isActiveWindow()) {
                    fmDebug() << "Window is active on X11, stopping check timer";
                    checkTimer.stop();
                } else {
                    fmDebug() << "Window lost focus on X11, starting check timer (300ms interval)";
                    //300ms检测一次
                    checkTimer.setInterval(300);
                    checkTimer.start();
                    QMetaObject::invokeMethod(this, "checkWindowOnX11", Qt::QueuedConnection);
                }
            });
            watchedWin = getRootWindow(x11Con, watchedWidget->winId());
        }
    } else {
        fmInfo() << "Stopping X11 window activation monitoring";
        q->disconnect(window, &QWindow::activeChanged, this, nullptr);
        checkTimer.stop();
    }
}

void AutoActivateWindowPrivate::checkWindowOnX11()
{
    if (!watchedWidget || !x11Con) {
        fmDebug() << "X11 check aborted: widget or connection is null";
        return;
    }

    if (watchedWidget->isActiveWindow()) {
        fmDebug() << "X11 check: window is already active";
        return;
    }

    xcb_query_tree_cookie_t cookie = xcb_query_tree(x11Con, rootWin);
    xcb_generic_error_t *err;
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(x11Con, cookie, &err);
    if (!reply) {
        fmWarning() << "can not get reply: xcb_query_tree";
        return;
    }

    if (err) {
        fmWarning() << "xcb_query_tree failed with error code " << err->error_code;
        free(reply);
        return;
    }

    xcb_window_t *children = xcb_query_tree_children(reply);
    int count = xcb_query_tree_children_length(reply);
    for (int i = count - 1; i > -1; i--) {
        xcb_get_window_attributes_reply_t *attrreply = xcb_get_window_attributes_reply(
                x11Con, xcb_get_window_attributes(x11Con, children[i]), nullptr);
        if (!attrreply)
            continue;
        bool viewable = attrreply->map_state == XCB_MAP_STATE_VIEWABLE;
        free(attrreply);

        if (children[i] == watchedWin) {
            // Use xcb_ set_ input_focus, there will be a problem that the outgoing initiator does not take away the focus
            //xcb_set_input_focus(m_x11Con, XCB_NONE, m_watchedWidget->winId(), XCB_CURRENT_TIME);
            watchedWidget->activateWindow();
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
    if (!x11Con) {
        fmDebug() << "Initializing X11 connection";
        int nbr;
        x11Con = xcb_connect(nullptr, &nbr);
        Q_ASSERT(x11Con);

        if (nbr < 0) {
            fmWarning() << "xcb_connect fail." << nbr;
            xcb_disconnect(x11Con);
            x11Con = nullptr;
            return false;
        }

        xcb_screen_t *screen = screen_of_display(x11Con, nbr);
        Q_ASSERT(screen);
        rootWin = screen->root;

        fmDebug() << "X11 connection established, screen number:" << nbr << "root window:" << rootWin;

        connect(&checkTimer, &QTimer::timeout, this, &AutoActivateWindowPrivate::checkWindowOnX11);
    }

    return true;
}

AutoActivateWindow::AutoActivateWindow(QObject *parent)
    : QObject(parent), d(new AutoActivateWindowPrivate(this))
{
}

void AutoActivateWindow::setWatched(QWidget *win)
{
    if (d->run) {
        fmWarning() << "Cannot set watched widget while auto-activate is running";
        return;
    }

    d->watchedWidget = win;
}

bool AutoActivateWindow::start()
{
    if (d->run) {
        fmWarning() << "AutoActivateWindow already started";
        return false;
    }
    fmInfo() << "Starting auto-activate window monitoring";

    //监控窗口状态
    if (WindowUtils::isWayLand()) {
        fmDebug() << "Using Wayland window monitoring";
        d->watchOnWayland(true);
    } else {
        fmDebug() << "Using X11 window monitoring";
        d->watchOnX11(true);
    }

    d->run = true;
    return true;
}

void AutoActivateWindow::stop()
{
    fmInfo() << "Stopping auto-activate window monitoring";

    d->run = false;
    if (WindowUtils::isWayLand()) {
        d->watchOnWayland(false);
    } else {
        d->watchOnX11(false);
    }
}
