/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaultmenuscene.h"
#include "vaultmenuscene_p.h"

#include "services/common/menu/menu_defines.h"

#include <QMenu>
#include <QList>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
using namespace dfmplugin_vault;

static const char *const kWorkSpaceScene = "WorkspaceMenu";

AbstractMenuScene *VaultMenuSceneCreator::create()
{
    return new VaultMenuScene();
}

VaultMenuScenePrivate::VaultMenuScenePrivate(VaultMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    menuServer = MenuService::service();
}

QStringList VaultMenuScenePrivate::emptyMenuActionRule()
{
    static QStringList actionRule {
        "new-folder",
        "new-document",
        "separator-line",
        "display-as",
        "sort-by",
        "refresh",
        "separator-line",
        "paste",
        "select-all",
        "property"
    };

    return actionRule;
}

QStringList VaultMenuScenePrivate::normalMenuActionRule()
{
    static QStringList actionRule {
        "open",
        "open-with",
        "separator-line",
        "open-in-new-window",
        "open-in-new-tab",
        "cut",
        "copy",
        "rename",
        "delete",
        "separator-line",
        "property"
    };

    return actionRule;
}

void VaultMenuScenePrivate::filterMenuAction(QMenu *menu, const QStringList &actions)
{
    QList<QAction *> actionlist = menu->actions();
    if (actionlist.isEmpty())
        return;

    QList<QAction *> removeActions;
    for (auto act : actionlist) {
        if (act->isSeparator())
            continue;
        QVariant actionId = act->property(ActionPropertyKey::kActionID);
        QString actionStr = actionId.toString();
        if (!actions.contains(actionStr)) {
            act->setVisible(false);
        }
    }

    menu->addActions(actionlist);
}

VaultMenuScene::VaultMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new VaultMenuScenePrivate(this))
{
}

QString VaultMenuScene::name() const
{
    return VaultMenuSceneCreator::name();
}

bool VaultMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (d->selectFiles.isEmpty() && d->currentDir.isValid()) {
        d->selectFiles << d->currentDir;
    }

    QList<AbstractMenuScene *> currentScene;
    if (auto workspaceScene = d->menuServer->createScene(kWorkSpaceScene))
        currentScene.append(workspaceScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *VaultMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    return AbstractMenuScene::scene(action);
}

bool VaultMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
}

void VaultMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
    if (d->isEmptyArea) {
        d->filterMenuAction(parent, d->emptyMenuActionRule());
    } else {
        d->filterMenuAction(parent, d->normalMenuActionRule());
    }
}

bool VaultMenuScene::triggered(QAction *action)
{
    return AbstractMenuScene::triggered(action);
}
