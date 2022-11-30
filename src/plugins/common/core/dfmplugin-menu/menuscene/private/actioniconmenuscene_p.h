// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACTIONICONMENUSCENEPRIVATE_H
#define ACTIONICONMENUSCENEPRIVATE_H

#include "menuscene/actioniconmenuscene.h"

#include "interfaces/private/abstractmenuscene_p.h"

DFMBASE_USE_NAMESPACE
DPMENU_BEGIN_NAMESPACE

class ActionIconMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class ActionIconMenuScene;

public:
    explicit ActionIconMenuScenePrivate(ActionIconMenuScene *qq);
};

DPMENU_END_NAMESPACE

#endif // ACTIONICONMENUSCENEPRIVATE_H
