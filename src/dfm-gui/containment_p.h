// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINMENT_P_H
#define CONTAINMENT_P_H

#include <dfm-gui/containment.h>
#include "applet_p.h"

DFMGUI_BEGIN_NAMESPACE

class ContainmentPrivate : public AppletPrivate
{
    Q_DECLARE_PUBLIC(Containment)

public:
    explicit ContainmentPrivate(Containment *q);

    QList<Applet *> applets;
};

DFMGUI_END_NAMESPACE

#endif   // CONTAINMENT_P_H
