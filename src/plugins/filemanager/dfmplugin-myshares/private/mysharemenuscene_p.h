// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MYSHAREMENUSCENE_P_H
#define MYSHAREMENUSCENE_P_H

#include "dfmplugin_myshares_global.h"
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_myshares {

class MyShareMenuScene;
class MyShareMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class MyShareMenuScene;

public:
    explicit MyShareMenuScenePrivate(AbstractMenuScene *qq);

private:
    void createFileMenu(QMenu *parent);
    bool triggered(const QString &id);
};

}

#endif   // MYSHAREMENUSCENE_P_H
