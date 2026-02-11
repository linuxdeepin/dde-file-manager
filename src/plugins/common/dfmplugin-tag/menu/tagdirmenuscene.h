// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDIRMENUSCENE_H
#define TAGDIRMENUSCENE_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_tag {

class TagDirMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "TagDirMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class TagDirMenuScenePrivate;
class TagDirMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit TagDirMenuScene(QObject *parent = nullptr);
    virtual ~TagDirMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<TagDirMenuScenePrivate> d;
};

}

#endif   // TAGDIRMENUSCENE_H
