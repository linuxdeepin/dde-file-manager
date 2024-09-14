// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTANDDISPLAYMENUSCENE_H
#define SORTANDDISPLAYMENUSCENE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_workspace {
class SortAndDisplayMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "SortAndDisplayMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class SortAndDisplayMenuScenePrivate;
class SortAndDisplayMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit SortAndDisplayMenuScene(QObject *parent = nullptr);
    virtual ~SortAndDisplayMenuScene() override;

    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    QScopedPointer<SortAndDisplayMenuScenePrivate> d;
};
}
#endif   // SORTANDDISPLAYMENUSCENE_H
