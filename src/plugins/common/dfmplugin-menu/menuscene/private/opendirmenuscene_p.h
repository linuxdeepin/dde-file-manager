// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENDIRMENUSCENE_P_H
#define OPENDIRMENUSCENE_P_H
#include "menuscene/opendirmenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class OpenDirMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class OpenDirMenuScene;
    explicit OpenDirMenuScenePrivate(OpenDirMenuScene *qq);
};

}

#endif   // OPENDIRMENUSCENE_P_H
