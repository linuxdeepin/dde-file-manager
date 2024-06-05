// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/panel.h>
#include "panel_p.h"

#include <QQuickWindow>

DFMGUI_BEGIN_NAMESPACE

PanelPrivate::PanelPrivate(Panel *q)
    : ContainmentPrivate(q)
{
    flag = Applet::kPanel;
}

/*!
 * \class Panel
 * \brief 面板/窗体，用于弹出式的 QML 窗体
 * \note Panel 的 componentUrl 需要指向基于 QQuickWindow 的 QML 组件
 */
Panel::Panel(QObject *parent)
    : Containment(*new PanelPrivate(this), parent)
{
}

/*!
 * \return 返回当前 Panel 关联的 QQuickWindow 窗体指针
 */
QQuickWindow *Panel::window() const
{
    return qobject_cast<QQuickWindow *>(d_func()->rootObject);
}

DFMGUI_END_NAMESPACE
