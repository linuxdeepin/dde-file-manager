/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#ifndef SENDTOMENUSCENE_P_H
#define SENDTOMENUSCENE_P_H

#include "dfmplugin_menu_global.h"

#include <interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {

class SendToMenuScene;
class SendToMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class SendToMenuScene;

public:
    explicit SendToMenuScenePrivate(SendToMenuScene *qq);

private:
    SendToMenuScene *q;

    QMultiHash<QString, QString> selectSupportActions;
};

}

#endif   // SENDTOMENUSCENE_P_H
