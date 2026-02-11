// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYMENUSCENE_P_H
#define PROPERTYMENUSCENE_P_H

#include "propertymenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_propertydialog {
DFMBASE_USE_NAMESPACE

class PropertyMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class PropertyMenuScene;
    explicit PropertyMenuScenePrivate(PropertyMenuScene *qq = nullptr);

private:
    void updateMenu(QMenu *menu);
};

}

#endif   // PROPERTYMENUSCENE_P_H
