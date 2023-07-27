// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultcomputermenuscene.h"
#include "vaultcomputermenuscene_p.h"
#include "utils/vaulthelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include "dfm-framework/dpf.h"

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

AbstractMenuScene *VaultComputerMenuCreator::create()
{
    return new VaultComputerMenuScene();
}

VaultComputerMenuScene::VaultComputerMenuScene(QObject *parent)
    : dfmbase::AbstractMenuScene(parent), d(new VaultComputerMenuScenePrivate(this))
{
}

VaultComputerMenuScene::~VaultComputerMenuScene()
{
}

QString VaultComputerMenuScene::name() const
{
    return VaultComputerMenuCreator::name();
}

bool VaultComputerMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (d->selectFiles.count() == 1 && d->selectFiles.first().path().endsWith(".vault"))
        return true;
    return false;
}

bool VaultComputerMenuScene::create(QMenu *parent)
{
    parent->clear();   // vault item in computer has its own menu actions.

    VaultHelper::instance()->appendWinID(d->windowId);
    auto menu = VaultHelper::instance()->createMenu();
    d->acts = menu->actions();
    parent->addActions(d->acts);
    menu->deleteLater();
    return true;
}

void VaultComputerMenuScene::updateState(QMenu *parent)
{
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
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<VaultComputerMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

VaultComputerMenuScenePrivate::VaultComputerMenuScenePrivate(VaultComputerMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}
