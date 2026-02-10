// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDCANVASSCENE_P_H
#define EXTENDCANVASSCENE_P_H

#include "ddplugin_organizer_global.h"

#include "extendcanvasscene.h"
#include "organizer_defines.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
namespace ddplugin_organizer {

class ExtendCanvasScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit ExtendCanvasScenePrivate(ExtendCanvasScene *qq);
    void emptyMenu(QMenu *parent);
    void normalMenu(QMenu *parent);

    void updateEmptyMenu(QMenu *parent);
    void updateNormalMenu(QMenu *parent);

    QMenu *organizeBySubActions(QMenu *menu);
    static QString classifierToActionID(Classifier cf);

    bool triggerSortby(const QString &actionId);

public:
    bool turnOn = false;
    bool onCollection = false;
    CollectionView *view = nullptr;

private:
    ExtendCanvasScene *q;
};

}

#endif   // EXTENDCANVASSCENE_P_H
