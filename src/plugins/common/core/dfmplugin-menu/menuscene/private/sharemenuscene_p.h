/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SHAREMENUSCENE_P_H
#define SHAREMENUSCENE_P_H

#include "menuscene/sharemenuscene.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class ShareMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class ShareMenuScene;

public:
    explicit ShareMenuScenePrivate(AbstractMenuScene *qq);

private:
    void addSubActions(QMenu *subMenu);
    void handleActionTriggered(QAction *act);

private:
    bool folderSelected { false };
};

}

#endif   // SHAREMENUSCENE_P_H
