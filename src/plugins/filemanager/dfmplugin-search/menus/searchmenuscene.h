// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHMENUSCENE_H
#define SEARCHMENUSCENE_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_search {

class SearchMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SearchMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SearchMenuScenePrivate;
class SearchMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit SearchMenuScene(QObject *parent = nullptr);
    ~SearchMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<SearchMenuScenePrivate> d;
};

}
#endif   // SEARCHMENUSCENE_H
