// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARDMENUSCENE_P_H
#define CLIPBOARDMENUSCENE_P_H
#include "menuscene/clipboardmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class ClipBoardMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class ClipBoardMenuScene;
    explicit ClipBoardMenuScenePrivate(AbstractMenuScene *qq);
    QList<QUrl> treeSelectedUrls;
};

}

#endif   // CLIPBOARDMENUSCENE_P_H
