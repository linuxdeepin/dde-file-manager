// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASESORTMENUSCENE_H
#define BASESORTMENUSCENE_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace ddplugin_canvas {

class CanvasBaseSortMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "CanvasBaseSortMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class CanvasBaseSortMenuScenePrivate;
class CanvasBaseSortMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT

public:
    explicit CanvasBaseSortMenuScene(QObject *parent = nullptr);
    virtual ~CanvasBaseSortMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<CanvasBaseSortMenuScenePrivate> d;
};

}
#endif   // BASESORTMENUSCENE_H
