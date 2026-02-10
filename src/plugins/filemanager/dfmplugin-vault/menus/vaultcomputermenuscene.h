// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCOMPUTERMENUSCENE_H
#define VAULTCOMPUTERMENUSCENE_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

namespace dfmplugin_vault {

class VaultComputerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
public:
    inline static QString name()
    {
        return "VaultComputerSubMenu";
    }

    virtual DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class VaultComputerMenuScenePrivate;
class VaultComputerMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
public:
    explicit VaultComputerMenuScene(QObject *parent = nullptr);
    virtual ~VaultComputerMenuScene() override;

    virtual QString name() const override;
    virtual bool initialize(const QVariantHash &params) override;
    virtual bool create(QMenu *parent) override;
    virtual void updateState(QMenu *parent) override;
    virtual bool triggered(QAction *action) override;
    virtual AbstractMenuScene *scene(QAction *action) const override;

private:
    QScopedPointer<VaultComputerMenuScenePrivate> d;
};

}

#endif   // VAULTCOMPUTERMENUSCENE_H
