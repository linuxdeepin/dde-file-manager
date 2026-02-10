// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIRSHAREMENUSCENE_P_H
#define DIRSHAREMENUSCENE_P_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_dirshare {
DFMBASE_USE_NAMESPACE

class DirShareMenuScene;
class DirShareMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class DirShareMenuScene;

public:
    explicit DirShareMenuScenePrivate(AbstractMenuScene *qq);

private:
    void addShare(const QUrl &url);
};

}

#endif   // DIRSHAREMENUSCENE_P_H
