// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHMENUSCENE_P_H
#define OPENWITHMENUSCENE_P_H

#include "menuscene/openwithmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class OpenWithMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class OpenWithMenuScene;
    explicit OpenWithMenuScenePrivate(OpenWithMenuScene *qq);
    QStringList recommendApps;
};

}

#endif   // OPENWITHMENUSCENE_P_H
