// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTMENUSCENE_H
#define VAULTMENUSCENE_H
#include "dfmplugin_vault_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_vault {

class VaultMenuSceneCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    static QString name()
    {
        return "VaultMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class VaultMenuScenePrivate;
class VaultMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit VaultMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;

private:
    VaultMenuScenePrivate *const d { nullptr };
};
}
#endif   // VAULTMENUSCENE_H
