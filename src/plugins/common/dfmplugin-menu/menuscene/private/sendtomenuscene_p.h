// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDTOMENUSCENE_P_H
#define SENDTOMENUSCENE_P_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

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
