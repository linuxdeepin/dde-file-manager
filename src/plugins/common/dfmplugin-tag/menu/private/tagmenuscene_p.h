// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMENUSCENE_P_H
#define TAGMENUSCENE_P_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_tag {

class TagMenuScene;
class TagMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class TagMenuScene;

public:
    explicit TagMenuScenePrivate(DFMBASE_NAMESPACE::AbstractMenuScene *qq);
    QRect getSurfaceRect(QWidget *);
    QStringList tagNames {};
    bool onCollection { false };
};

}

#endif   // TAGMENUSCENE_P_H
