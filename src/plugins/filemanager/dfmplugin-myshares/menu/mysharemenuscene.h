// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MYSHAREMENUSCENE_H
#define MYSHAREMENUSCENE_H

#include "dfmplugin_myshares_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_myshares {

class MyShareMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "MyShareMenu";
    }

    virtual dfmbase::AbstractMenuScene *create() override;
};

class MyShareMenuScenePrivate;
class MyShareMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit MyShareMenuScene(QObject *parent = nullptr);
    virtual ~MyShareMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual dfmbase::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<MyShareMenuScenePrivate> d;
};

}
#endif   // MYSHAREMENUSCENE_H
