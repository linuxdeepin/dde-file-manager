// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGHIDDENMENUSCENE_P_H
#define DCONFIGHIDDENMENUSCENE_P_H

#include "menuscene/dconfighiddenmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
DPMENU_BEGIN_NAMESPACE

class DConfigHiddenMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class DConfigHiddenMenuScene;

public:
    explicit DConfigHiddenMenuScenePrivate(DConfigHiddenMenuScene *qq);
};

DPMENU_END_NAMESPACE

#endif   // DCONFIGHIDDENMENUSCENE_P_H
