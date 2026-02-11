// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERMENUSCENE_H
#define COMPUTERMENUSCENE_H

#include "dfmplugin_computer_global.h"
#include "utils/computerutils.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return ComputerUtils::menuSceneName();
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ComputerMenuScenePrivate;
class ComputerMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit ComputerMenuScene(QObject *parent = nullptr);
    virtual ~ComputerMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual dfmbase::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<ComputerMenuScenePrivate> d;
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERMENUSCENE_H
