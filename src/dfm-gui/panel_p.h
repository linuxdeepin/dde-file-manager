// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_P_H
#define PANEL_P_H

#include <dfm-gui/panel.h>
#include "containment_p.h"

DFMGUI_BEGIN_NAMESPACE

class PanelPrivate : public ContainmentPrivate
{
    Q_DECLARE_PUBLIC(Panel)

public:
    explicit PanelPrivate(Panel *q);

    void setRootObject(QObject *item) override;
};

DFMGUI_END_NAMESPACE

#endif   // PANEL_P_H
