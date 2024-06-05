// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETITEM_P_H
#define APPLETITEM_P_H

#include "dfm-gui/appletitem.h"

DFMGUI_BEGIN_NAMESPACE

class AppletItemPrivate
{
    Q_DECLARE_PUBLIC(AppletItem)
    Q_DISABLE_COPY(AppletItemPrivate)

public:
    explicit AppletItemPrivate(AppletItem *q);

    QPointer<AppletItem> q_ptr;
    Applet *applet = nullptr;
};

DFMGUI_END_NAMESPACE

#endif   // APPLETITEM_P_H
