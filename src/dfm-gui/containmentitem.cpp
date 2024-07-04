// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/containmentitem.h>
#include <dfm-gui/containment.h>

DFMGUI_BEGIN_NAMESPACE

/*!
 * \class ContainmentItem
 * \brief Containment 对应的 Quick 组件，通过 Applet componentUrl 创建
 */
ContainmentItem::ContainmentItem(QQuickItem *parent)
    : AppletItem(parent)
{
}

ContainmentItem::~ContainmentItem() { }

AppletItem *ContainmentItem::itemFor(Applet *applet)
{
    if (applet && applet->flags().testFlag(Applet::kContainment)) {
        return AppletItem::itemForApplet(applet);
    }

    return nullptr;
}

DFMGUI_END_NAMESPACE
