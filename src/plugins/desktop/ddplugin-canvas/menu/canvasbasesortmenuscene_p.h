// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASESORTMENUSCENE_P_H
#define BASESORTMENUSCENE_P_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DDP_CANVAS_BEGIN_NAMESPACE

class CanvasBaseSortMenuScene;
class CanvasBaseSortMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class CanvasBaseSortMenuScene;

public:
    explicit CanvasBaseSortMenuScenePrivate(CanvasBaseSortMenuScene *qq);

    void sortPrimaryMenu(QMenu *menu);
    void sortSecondaryMenu(QMenu *menu);

    QStringList sendToRule();
    QStringList stageToRule();

    QStringList primaryMenuRule();
    QMap<QString, QStringList> secondaryMenuRule();

private:
    CanvasBaseSortMenuScene *q;
};

DDP_CANVAS_END_NAMESPACE

#endif   // BASESORTMENUSCENE_P_H
