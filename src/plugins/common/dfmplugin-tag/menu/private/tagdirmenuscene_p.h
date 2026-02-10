// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDIRMENUSCENEPRIVATE_H
#define TAGDIRMENUSCENEPRIVATE_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_tag {

class TagDirMenuScene;
class TagDirMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class TagDirMenuScene;

public:
    explicit TagDirMenuScenePrivate(TagDirMenuScene *qq);
    bool openFileLocation(const QString &path);
    void updateMenu(QMenu *menu);

private:
    TagDirMenuScene *q;
};

}

#endif   // TAGDIRMENUSCENEPRIVATE_H
