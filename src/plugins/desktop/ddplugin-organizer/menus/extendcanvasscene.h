// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDCANVASSCENE_H
#define EXTENDCANVASSCENE_H

#include "ddplugin_organizer_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace ddplugin_organizer {

class CollectionView;
class ExtendCanvasCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OrganizerExtCanvasMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class ExtendCanvasScenePrivate;
class ExtendCanvasScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT

public:
    explicit ExtendCanvasScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;
    bool actionFilter(AbstractMenuScene *caller, QAction *action) override;

private:
    ExtendCanvasScenePrivate *const d = nullptr;
};

}

#endif // EXTENDCANVASSCENE_H
