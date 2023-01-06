// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTOACTIVATEWINDOW_P_H
#define AUTOACTIVATEWINDOW_P_H

#include "autoactivatewindow.h"

#include <QTimer>

#include <xcb/xcb.h>

class AutoActivateWindowPrivate : public QObject
{
    Q_OBJECT
public:
    explicit AutoActivateWindowPrivate(AutoActivateWindow *parent);
    ~AutoActivateWindowPrivate();
public:
    AutoActivateWindow *q;
    bool m_run = false;
    QWidget *m_watchedWidget = nullptr;
public: //wayland
    void watchOnWayland(bool on);
public: //x11
    void watchOnX11(bool on);
protected slots:
    void checkWindowOnX11();
private:
    bool initConnect();
    xcb_connection_t *m_x11Con = nullptr;
    xcb_window_t m_rootWin = {0};
    xcb_window_t m_watchedWin = {0};
    QTimer m_checkTimer;
};

#endif // AUTOACTIVATEWINDOW_P_H
