// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTANDDISPLAYMENUSCENE_P_H
#define SORTANDDISPLAYMENUSCENE_P_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_workspace {

class FileView;
class SortAndDisplayMenuScene;
class SortAndDisplayMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend SortAndDisplayMenuScene;

public:
    explicit SortAndDisplayMenuScenePrivate(AbstractMenuScene *qq);

private:
    void createEmptyMenu(QMenu *parent);
    QMenu *addSortByActions(QMenu *menu);
    QMenu *addDisplayAsActions(QMenu *menu);

    void sortByRole(int role);
    void updateEmptyAreaActionState();

private:
    FileView *view = nullptr;
};

}

#endif   // SORTANDDISPLAYMENUSCENE_P_H
