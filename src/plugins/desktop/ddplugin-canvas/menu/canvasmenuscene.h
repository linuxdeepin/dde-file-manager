// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMENUSCENE_H
#define CANVASMENUSCENE_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace ddplugin_canvas {

class CanvasMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "CanvasMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class CanvasMenuScenePrivate;
class CanvasMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit CanvasMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    void emptyMenu(QMenu *parent);
    void normalMenu(QMenu *parent);
    QMenu *sortBySubActions(QMenu *menu);
    QMenu *iconSizeSubActions(QMenu *menu);

private:
    CanvasMenuScenePrivate *const d = nullptr;
};

}
#endif   // CANVASMENUSCENE_H
