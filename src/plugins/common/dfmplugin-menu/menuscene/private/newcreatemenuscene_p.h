// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWCREATEMENUSCENE_P_H
#define NEWCREATEMENUSCENE_P_H
#include "menuscene/newcreatemenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class NewCreateMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class NewCreateMenuScene;
    explicit NewCreateMenuScenePrivate(NewCreateMenuScene *qq);
};

}

#endif   // NEWCREATEMENUSCENE_P_H
