// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-gui/windowhandle.h>
#include <QObject>

class QQuickWindow;
class QQmlEngine;

DFMGUI_BEGIN_NAMESPACE

class WindowManagerPrivate;
class WindowManager : public QObject
{
    Q_OBJECT

    explicit WindowManager();
    ~WindowManager() override;

public:
    static WindowManager *instance();
    QSharedPointer<QQmlEngine> engine() const;

    WindowHandlePtr createWindow(const QString &pluginName, const QString &quickId,
                                 const QVariantMap &var = {});
    void showWindow(const WindowHandlePtr &handle);

private:
    QScopedPointer<WindowManagerPrivate> dptr;
    Q_DECLARE_PRIVATE_D(dptr, WindowManager);
    Q_DISABLE_COPY(WindowManager)
};

DFMGUI_END_NAMESPACE

#define FMQuickWindowIns DFMGUI_NAMESPACE::WindowManager::instance();

#endif   // WINDOWMANAGER_H
