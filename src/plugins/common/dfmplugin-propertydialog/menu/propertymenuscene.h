// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROPERTYMENUSCENE_H
#define PROPERTYMENUSCENE_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_propertydialog {

class PropertyMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "PropertyMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class PropertyMenuScenePrivate;
class PropertyMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit PropertyMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    PropertyMenuScenePrivate *const d = nullptr;
};

}

#endif   // PROPERTYMENUSCENE_H
