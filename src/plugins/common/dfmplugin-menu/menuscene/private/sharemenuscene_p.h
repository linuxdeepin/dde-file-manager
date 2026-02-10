// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREMENUSCENE_P_H
#define SHAREMENUSCENE_P_H

#include "menuscene/sharemenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class ShareMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class ShareMenuScene;

public:
    explicit ShareMenuScenePrivate(AbstractMenuScene *qq);
};

}

#endif   // SHAREMENUSCENE_P_H
