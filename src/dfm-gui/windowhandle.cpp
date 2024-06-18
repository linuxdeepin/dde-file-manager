// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/windowhandle.h>
#include <dfm-gui/panel.h>

#include <QQuickWindow>

DFMGUI_BEGIN_NAMESPACE

class WindowHandleData : public QSharedData
{
public:
    QPointer<Panel> panel;
    QPointer<QQuickWindow> window;
    QString errorString;
};

/*!
 * \class WindowHandle
 * \brief 提供用于操作的窗口和Applet指针
 */
WindowHandle::WindowHandle()
    : data(new WindowHandleData)
{
}

WindowHandle::WindowHandle(const QString &error)
    : data(new WindowHandleData)
{
    data->errorString = error;
}

WindowHandle::WindowHandle(Panel *p, QQuickWindow *w)
    : data(new WindowHandleData)
{
    data->panel = p;
    data->window = w;
}

WindowHandle::~WindowHandle() { }

bool WindowHandle::isValid() const
{
    return data->window && data->panel;
}

QPointer<QQuickWindow> WindowHandle::window() const
{
    return data->window;
}

QPointer<Panel> WindowHandle::panel() const
{
    return data->panel;
}

QString WindowHandle::lastError() const
{
    return data->errorString;
}

DFMGUI_END_NAMESPACE
