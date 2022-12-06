// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMENUSCENE_P_H
#define TEMPLATEMENUSCENE_P_H

#include "templatemenuscene/templatemenuscene.h"

#include "interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class TemplateMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class TemplateMenuScene;
    explicit TemplateMenuScenePrivate(AbstractMenuScene *qq);

    QList<QAction *> templateActions;
};

}

#endif   // TEMPLATEMENUSCENE_P_H
