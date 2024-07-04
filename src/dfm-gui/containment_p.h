// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINMENT_P_H
#define CONTAINMENT_P_H

#include <dfm-gui/containment.h>
#include "applet_p.h"

#include <QQuickWindow>

DFMGUI_BEGIN_NAMESPACE

class ContainmentPrivate : public AppletPrivate
{
    Q_DECLARE_PUBLIC(Containment)

public:
    explicit ContainmentPrivate(Containment *q);

    void setRootObject(QObject *item) override;

    QList<Applet *> applets;
};

DFMGUI_END_NAMESPACE

#endif   // CONTAINMENT_P_H
