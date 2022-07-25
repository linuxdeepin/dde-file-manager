/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TAGDIRMENUSCENEPRIVATE_H
#define TAGDIRMENUSCENEPRIVATE_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

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
