// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHMENUSCENE_P_H
#define TRASHMENUSCENE_P_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_trash {

class TrashMenuScene;
class TrashMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class TrashMenuScene;

public:
    explicit TrashMenuScenePrivate(TrashMenuScene *qq);

    void updateMenu(QMenu *menu);
    void updateSubMenu(QMenu *menu);

private:
    TrashMenuScene *q;

    QMultiHash<QString, QString> selectSupportActions;
};

}

#endif   // TRASHMENUSCENE_P_H
