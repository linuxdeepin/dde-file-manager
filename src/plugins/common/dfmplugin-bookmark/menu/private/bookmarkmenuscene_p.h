// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKMENUSCENE_P_H
#define BOOKMARKMENUSCENE_P_H

#include "dfmplugin_bookmark_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_bookmark {

class BookmarkMenuScene;
class BookmarkMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class BookmarkMenuScene;

public:
    explicit BookmarkMenuScenePrivate(DFMBASE_NAMESPACE::AbstractMenuScene *qq);

    bool showBookMarkMenu = true;
};

}

#endif   // BOOKMARKMENUSCENE_P_H
