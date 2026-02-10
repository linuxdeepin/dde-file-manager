// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultcomputermenuscene.h"
#include "vaultcomputermenuscene_p.h"
#include "utils/vaulthelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-framework/dpf.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

AbstractMenuScene *VaultComputerMenuCreator::create()
{
    fmDebug() << "Vault: Creating VaultComputerMenuScene";
    return new VaultComputerMenuScene();
}

VaultComputerMenuScene::VaultComputerMenuScene(QObject *parent)
    : dfmbase::AbstractMenuScene(parent), d(new VaultComputerMenuScenePrivate(this))
{
    fmDebug() << "Vault: VaultComputerMenuScene initialized";
}

VaultComputerMenuScene::~VaultComputerMenuScene()
{
    fmDebug() << "Vault: VaultComputerMenuScene destroyed";
}

QString VaultComputerMenuScene::name() const
{
    return VaultComputerMenuCreator::name();
}

bool VaultComputerMenuScene::initialize(const QVariantHash &params)
{
    fmDebug() << "Vault: Initializing menu scene with params count:" << params.size();

    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (d->selectFiles.count() == 1 && d->selectFiles.first().path().endsWith(".vault"))
        return true;

    fmDebug() << "Vault: Invalid selection for vault menu scene";
    return false;
}

bool VaultComputerMenuScene::create(QMenu *parent)
{
    fmDebug() << "Vault: Creating vault computer menu";

    if (!parent) {
        fmWarning() << "Vault: Parent menu is null";
        return false;
    }

    parent->clear();   // vault item in computer has its own menu actions.

    VaultHelper::instance()->appendWinID(d->windowId);
    auto menu = VaultHelper::instance()->createMenu();
    if (!menu) {
        fmWarning() << "Vault: Failed to create vault menu";
        return false;
    }

    d->acts = menu->actions();
    parent->addActions(d->acts);
    menu->deleteLater();
    return true;
}

void VaultComputerMenuScene::updateState(QMenu *parent)
{
    fmDebug() << "Vault: Updating menu state";
    return AbstractMenuScene::updateState(parent);
}

bool VaultComputerMenuScene::triggered(QAction *action)
{
    if (d->acts.contains(action))
        return true;

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *VaultComputerMenuScene::scene(QAction *action) const
{
    fmDebug() << "Vault: Getting scene for action:" << (action ? action->text() : "null");

    if (action == nullptr) {
        fmDebug() << "Vault: Action is null, returning null scene";
        return nullptr;
    }

    if (!d->predicateAction.key(action).isEmpty()) {
        fmDebug() << "Vault: Action belongs to vault menu scene";
        return const_cast<VaultComputerMenuScene *>(this);
    }

    return AbstractMenuScene::scene(action);
}

VaultComputerMenuScenePrivate::VaultComputerMenuScenePrivate(VaultComputerMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    fmDebug() << "Vault: VaultComputerMenuScenePrivate initialized";
}
