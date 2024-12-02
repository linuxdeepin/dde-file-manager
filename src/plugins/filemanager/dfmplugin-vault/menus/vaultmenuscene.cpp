// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultmenuscene.h"
#include "vaultmenuscene_p.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QMenu>
#include <QList>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

static const char *const kWorkSpaceScene = "WorkspaceMenu";

AbstractMenuScene *VaultMenuSceneCreator::create()
{
    return new VaultMenuScene();
}

VaultMenuScenePrivate::VaultMenuScenePrivate(VaultMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
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
        "stage-file-to-burning",
        "cut",
        "copy",
        "rename",
        "delete",
        "reverse-select",
        "separator-line",
        "send-to",
        "property"
    };

    return actionRule;
}

void VaultMenuScenePrivate::filterMenuAction(QMenu *menu, const QStringList &actions)
{
    QList<QAction *> actionlist = menu->actions();
    if (actionlist.isEmpty())
        return;

    for (auto act : actionlist) {
        if (act->isSeparator())
            continue;
        QVariant actionId = act->property(ActionPropertyKey::kActionID);
        QString actionStr = actionId.toString();
        if (actionStr == dfmplugin_menu::ActionID::kSendTo) {
            auto subMenu = act->menu();
            if (subMenu) {
                int invisibleActionNum { 0 };
                int numOfSeparator { 0 };
                for (QAction *action : subMenu->actions()) {
                    const QString &actionID = action->property(ActionPropertyKey::kActionID).toString();
                    if (actionID == dfmplugin_menu::ActionID::kSeparator) {
                        numOfSeparator++;
                    } else if (actionID == dfmplugin_menu::ActionID::kSendToDesktop
                               || actionID == dfmplugin_menu::ActionID::kCreateSymlink) {
                        action->setVisible(false);
                        invisibleActionNum++;
                    }
                }
                if ((invisibleActionNum + numOfSeparator) == subMenu->actions().count())
                    act->setVisible(false);
            }
        }
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
        fmWarning() << "Vault: menu scene:" << name() << " init failed.";
        return false;
    }

    if (d->selectFiles.isEmpty() && d->currentDir.isValid()) {
        d->selectFiles << d->currentDir;
    }

    QList<AbstractMenuScene *> currentScene;
    if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkSpaceScene))
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
