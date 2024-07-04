// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEMENUSCENE_H
#define WORKSPACEMENUSCENE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_workspace {

class WorkspaceMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "WorkspaceMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class WorkspaceMenuScenePrivate;
class WorkspaceMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit WorkspaceMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    bool emptyMenuTriggered(QAction *action);
    bool normalMenuTriggered(QAction *action);

private:
    WorkspaceMenuScenePrivate *const d = nullptr;
};

}
#endif   // WORKSPACEMENUSCENE_H
