// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEMENUSCENE_P_H
#define TEMPLATEMENUSCENE_P_H

#include "templatemenuscene/templatemenuscene.h"
#include "templatemenuscene/templatemenu.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {

class TemplateMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit TemplateMenuScenePrivate(TemplateMenuScene *qq);
public:
    QList<QAction *> templateActions;
};

}

#endif   // TEMPLATEMENUSCENE_P_H
