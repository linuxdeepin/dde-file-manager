// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWMANAGER_P_H
#define WINDOWMANAGER_P_H

#include <dfm-gui/windowmanager.h>
#include <dfm-gui/applet.h>
#include <dfm-gui/sharedqmlengine.h>

DFMGUI_BEGIN_NAMESPACE

class WindowManagerPrivate
{
    Q_DECLARE_PUBLIC(WindowManager)
public:
    explicit WindowManagerPrivate(WindowManager *q);
    ~WindowManagerPrivate() = default;

    void asyncLoadQuickItem(Applet *applet);
    void aboutToQuit();

    QPointer<WindowManager> q_ptr;
    QScopedPointer<SharedQmlEngine> globalEngine;   // 用于资源管理的 QmlEngine 对象
    QMap<QQuickWindow *, WindowHandlePtr> windows;   // 当前已创建的 Window 和 Panel
};

DFMGUI_END_NAMESPACE

#endif   // WINDOWMANAGER_P_H
