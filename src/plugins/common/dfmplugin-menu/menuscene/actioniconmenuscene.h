// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONICONMENUSCENE_H
#define ACTIONICONMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

DPMENU_BEGIN_NAMESPACE

class ActionIconMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ActionIconManager";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ActionIconMenuScene: public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit ActionIconMenuScene(QObject *parent = nullptr);

    virtual QString name() const override;
    virtual void updateState(QMenu *parent) override;

    bool actionIconVisible() const;
};

DPMENU_END_NAMESPACE

#endif // ACTIONICONMENUSCENE_H
