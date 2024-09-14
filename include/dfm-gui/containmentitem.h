// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINMENTITEM_H
#define CONTAINMENTITEM_H

#include <dfm-gui/appletitem.h>

DFMGUI_BEGIN_NAMESPACE

class ContainmentItem : public AppletItem
{
    Q_OBJECT

public:
    ContainmentItem(QQuickItem *parent = nullptr);
    ~ContainmentItem() override;

    Q_INVOKABLE AppletItem *itemFor(Applet *applet);
};

DFMGUI_END_NAMESPACE

#endif   // CONTAINMENTITEM_H
