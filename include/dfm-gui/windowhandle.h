// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWHANDLE_H
#define WINDOWHANDLE_H

#include <dfm-gui/dfm_gui_global.h>

#include <QObject>
#include <QSharedDataPointer>

class QQuickWindow;

DFMGUI_BEGIN_NAMESPACE

class Panel;
class WindowHandleData;

class WindowHandle
{
public:
    explicit WindowHandle();
    explicit WindowHandle(const QString &error);
    WindowHandle(Panel *p, QQuickWindow *w);
    ~WindowHandle();

    bool isValid() const;
    QPointer<QQuickWindow> window() const;
    QPointer<Panel> panel() const;
    QString lastError() const;

private:
    Q_DISABLE_COPY(WindowHandle);
    QScopedPointer<WindowHandleData> data;
};

using WindowHandlePtr = QSharedPointer<WindowHandle>;

DFMGUI_END_NAMESPACE

#endif   // WINDOWHANDLE_H
