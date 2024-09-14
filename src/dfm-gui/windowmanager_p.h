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

    void registerType(const char *uri);

    WindowManager::Handle createQuickWindow(const QString &pluginName, const QString &quickId, const QVariantMap &var);
    void connectWindowHandle(const WindowManager::Handle &handle);

    bool isValidUrl(const QUrl &url, QString &error) const;
    bool compareUrl(const QUrl &cur, const QUrl &other) const;
    void asyncLoadQuickItem(Applet *applet);

    void moveWindowToCenter(QQuickWindow *window);
    void windowClosed(quint64 windId);
    void aboutToQuit();

    QPointer<WindowManager> q_ptr;
    QScopedPointer<SharedQmlEngine> globalEngine;   // 用于资源管理的 QmlEngine 对象
    QMap<quint64, WindowManager::Handle> windows;   // 当前已创建的 Window 和 Panel 并管理其生命周期
    quint64 previousActivedWindowId = 0;   // 之前的焦点窗口ID，用于切换界面时状态更新
    QString lastError;
};

DFMGUI_END_NAMESPACE

#endif   // WINDOWMANAGER_P_H
