// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OEMMENUSCENE_P_H
#define OEMMENUSCENE_P_H

#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {

class OemMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    explicit OemMenuScenePrivate(OemMenuScene *qq);

    QList<QAction *> childActions(QAction *action);

public:
    OemMenu *oemMenu = nullptr;

    QList<QAction *> oemActions;
    QList<QAction *> oemChildActions;
};

}

#endif   // OEMMENUSCENE_P_H
