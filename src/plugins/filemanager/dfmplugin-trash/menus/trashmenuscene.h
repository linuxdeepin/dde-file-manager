// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHMENUSCENE_H
#define TRASHMENUSCENE_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_trash {

class TrashMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "TrashMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class TrashMenuScenePrivate;
class TrashMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit TrashMenuScene(QObject *parent = nullptr);
    virtual ~TrashMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<TrashMenuScenePrivate> d;
};

}
#endif   // TRASHMENUSCENE_H
