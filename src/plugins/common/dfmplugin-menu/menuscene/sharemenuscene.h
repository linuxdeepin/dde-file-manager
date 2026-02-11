// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREMENUSCENE_H
#define SHAREMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_menu {

class ShareMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "ShareMenu";
    }

    virtual dfmbase::AbstractMenuScene *create() override;
};

class ShareMenuScenePrivate;
class ShareMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit ShareMenuScene(QObject *parent = nullptr);
    virtual ~ShareMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual dfmbase::AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<ShareMenuScenePrivate> d;
};

}

#endif   // SHAREMENUSCENE_H
