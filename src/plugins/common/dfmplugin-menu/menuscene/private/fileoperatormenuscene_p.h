// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATORMENUSCENE_P_H
#define FILEOPERATORMENUSCENE_P_H

#include "menuscene/fileoperatormenuscene.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_menu {
DFMBASE_USE_NAMESPACE

class FileOperatorMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    friend class FileOperatorMenuScene;
    explicit FileOperatorMenuScenePrivate(FileOperatorMenuScene *qq);
    QList<QUrl> treeSelectedUrls;
};

}

#endif   // FILEOPERATORMENUSCENE_P_H
