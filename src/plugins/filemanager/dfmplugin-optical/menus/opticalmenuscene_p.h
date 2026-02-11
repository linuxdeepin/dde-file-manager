// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALMENUSCENE_P_H
#define OPTICALMENUSCENE_P_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_optical {

class OpticalMenuScene;
class OpticalMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class OpticalMenuScene;

public:
    explicit OpticalMenuScenePrivate(OpticalMenuScene *qq);
    QString findSceneName(QAction *act) const;
    bool enablePaste() const;

private:
    OpticalMenuScene *q;
    bool isBlankDisc { false };
};

}

#endif   // OPTICALMENUSCENE_P_H
