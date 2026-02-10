// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSMENUSCENE_P_H
#define AVFSMENUSCENE_P_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_avfsbrowser {

class AvfsMenuScene;
class AvfsMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT

    friend class AvfsMenuScene;

    bool showOpenWith { false };

public:
    explicit AvfsMenuScenePrivate(AvfsMenuScene *qq);
};

}
#endif   // AVFSMENUSCENE_P_H
