// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASESORTMENUSCENE_P_H
#define BASESORTMENUSCENE_P_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_workspace {

class BaseSortMenuScene;
class BaseSortMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class BaseSortMenuScene;

public:
    explicit BaseSortMenuScenePrivate(BaseSortMenuScene *qq);

    void sortMenuActions(QMenu *menu, const QStringList &sortRule, bool isFuzzy);
    void sortPrimaryMenu(QMenu *menu);
    void sortSecondaryMenu(QMenu *menu);

    QStringList sendToRule();
    QStringList stageToRule();

    QStringList primaryMenuRule();
    QMap<QString, QStringList> secondaryMenuRule();

private:
    BaseSortMenuScene *q;
};

}

#endif   // BASESORTMENUSCENE_P_H
