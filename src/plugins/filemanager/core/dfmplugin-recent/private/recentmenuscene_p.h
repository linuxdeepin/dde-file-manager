// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMENUSCENE_P_H
#define RECENTMENUSCENE_P_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_recent {

class RecentMenuScene;
class RecentMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class RecentMenuScene;

public:
    explicit RecentMenuScenePrivate(RecentMenuScene *qq);

    void updateMenu(QMenu *menu);
    void updateSubMenu(QMenu *menu);
    void disableSubScene(DFMBASE_NAMESPACE::AbstractMenuScene *scene, const QString &sceneName);

private:
    RecentMenuScene *q;

    QMultiHash<QString, QString> selectDisableActions;
    QMultiHash<QString, QString> emptyDisableActions;
};

}

#endif   // RECENTMENUSCENE_P_H
