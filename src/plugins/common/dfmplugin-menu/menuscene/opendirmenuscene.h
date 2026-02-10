// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENDIRMENUSCENE_H
#define OPENDIRMENUSCENE_H

#include "dfmplugin_menu_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_menu {

class OpenDirMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "OpenDirMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class OpenDirMenuScenePrivate;
class OpenDirMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit OpenDirMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    void emptyMenu(QMenu *parent);
    void normalMenu(QMenu *parent);
    void openAsAdminAction(QMenu *parent);

private:
    OpenDirMenuScenePrivate *const d = nullptr;
};

}
#endif   // OPENDIRMENUSCENE_H
