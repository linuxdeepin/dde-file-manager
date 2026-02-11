// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSMENUSCENE_H
#define AVFSMENUSCENE_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_avfsbrowser {

class AvfsMenuSceneCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return "AvfsMenu";
    }

    // AbstractSceneCreator interface
public:
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class AvfsMenuScenePrivate;
class AvfsMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit AvfsMenuScene(QObject *parent = nullptr);
    virtual ~AvfsMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual DFMBASE_NAMESPACE::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<AvfsMenuScenePrivate> d;
};

}

#endif   // AVFSMENUSCENE_H
