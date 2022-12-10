/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "vaultcomputermenuscene.h"
#include "vaultcomputermenuscene_p.h"
#include "utils/vaulthelper.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-framework/dpf.h"

#include <QMenu>

using namespace dfmplugin_vault;
DFMBASE_USE_NAMESPACE

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
    if (d->acts.contains(action)) {
        dpfSlotChannel->push("dfmplugin_utils", "slot_ReportLog_ReportMenuData", action->text(), d->selectFiles);
        return true;
    }
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
