// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OEMMENUSCENE_P_H
#define OEMMENUSCENE_P_H

#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {

class OemMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    explicit OemMenuScenePrivate(OemMenuScene *qq);

    QList<QAction *> childActions(QAction *action);

public:
    OemMenu *oemMenu = nullptr;

    QList<QAction *> oemActions;
    QList<QAction *> oemChildActions;

    // External extensions may not know the meaning of the url inside the DFM
    QUrl transformedCurrentDir;
    QList<QUrl> transformedSelectFiles;
    QUrl transformedFocusFile;
};

}

#endif   // OEMMENUSCENE_P_H
