// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDMENUSCENE_P_H
#define EXTENDMENUSCENE_P_H

#include "extendmenuscene/extendmenuscene.h"
#include "extendmenuscene/extendmenu/dcustomactiondefine.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {

class ExtendMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    explicit ExtendMenuScenePrivate(ExtendMenuScene *qq);
    QList<QAction *> childActions(QAction *action);
    int mayComboPostion(const DCustomActionData &acdata, DCustomActionDefines::ComboType combo);
public:
    DCustomActionParser *customParser = nullptr;
    QList<QAction *> extendActions;
    QList<QAction *> extendChildActions;

    QMap<int, QList<QAction *>> cacheLocateActions;
    QMap<QAction *, DCustomActionDefines::Separator> cacheActionsSeparator;

    // External extensions may not know the meaning of the url inside the DFM
    QUrl transformedCurrentDir;
    QList<QUrl> transformedSelectFiles;
    QUrl transformedFocusFile;
};

}

#endif   // EXTENDMENUSCENE_P_H
