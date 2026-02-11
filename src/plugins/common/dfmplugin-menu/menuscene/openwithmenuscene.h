// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHMENUSCENE_H
#define OPENWITHMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_menu {

class OpenWithMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OpenWithMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class OpenWithMenuScenePrivate;
class OpenWithMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit OpenWithMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    OpenWithMenuScenePrivate *const d = nullptr;
};

}

#endif   // OPENWITHMENUSCENE_H
