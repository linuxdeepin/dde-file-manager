// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHREDMENUSCENE_P_H
#define SHREDMENUSCENE_P_H

#include "shredmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_utils {

class ShredMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit ShredMenuScenePrivate(ShredMenuScene *qq = nullptr);

    void updateMenu(QMenu *menu);
};

}

#endif   // SHREDMENUSCENE_P_H
