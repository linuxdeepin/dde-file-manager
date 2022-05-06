/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#include "recentmenuscene.h"
#include "private/recentmenuscene_p.h"
#include "utils/recentfileshelper.h"
#include "utils/recentfileshelper.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/menu/menuservice.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include "dfm-base/base/device/devicecontroller.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QMenu>

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";
static constexpr char kFileOperatorMenuSceneName[] = "FileOperatorMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kOpenDirMenuSceneName[] = "OpenDirMenu";

DPRECENT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *RecentMenuCreator::create()
{
    return new RecentMenuScene();
}

RecentMenuScene::RecentMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new RecentMenuScenePrivate(this))
{
}

RecentMenuScene::~RecentMenuScene()
{
}

QString RecentMenuScene::name() const
{
    return RecentMenuCreator::name();
}

bool RecentMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE

    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;
    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
        if (auto workspaceScene = MenuService::service()->createScene(kWorkspaceMenuSceneName))
            currentScene.append(workspaceScene);
    } else {
        if (auto workspaceScene = MenuService::service()->createScene(kSortAndDisplayMenuSceneName))
            currentScene.append(workspaceScene);

        if (auto workspaceScene = MenuService::service()->createScene(kOpenDirMenuSceneName))
            currentScene.append(workspaceScene);
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

bool RecentMenuScene::create(QMenu *parent)
{
    DSC_USE_NAMESPACE

    if (!d->isEmptyArea) {
        auto act = parent->addAction(d->predicateName[RecentActionID::kRemove]);
        act->setProperty(ActionPropertyKey::kActionID, RecentActionID::kRemove);
        d->predicateAction[RecentActionID::kRemove] = act;
    }

    return AbstractMenuScene::create(parent);
}

void RecentMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
    d->updateMenu(parent);
}

bool RecentMenuScene::triggered(QAction *action)
{
    DSC_USE_NAMESPACE
    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actId)) {
        if (actId == RecentActionID::kRemove) {
            RecentFilesHelper::removeRecent(d->selectFiles);
            return true;
        }
        qWarning() << "action not found, id: " << actId;
        return false;
    } else {
        return AbstractMenuScene::triggered(action);
    }
}

AbstractMenuScene *RecentMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<RecentMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

RecentMenuScenePrivate::RecentMenuScenePrivate(RecentMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
    predicateName[RecentActionID::kRemove] = tr("Remove");

    selectDisableActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kPaste);
    selectDisableActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCut);
    selectDisableActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kRename);
    selectDisableActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kDelete);

    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenAsAdmin);
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewTab);
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInTerminal);
}

void RecentMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    if (isEmptyArea) {
        QString sceneNameCurrent;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene) {
                // no scene,remove it.
                menu->removeAction(act);
                continue;
            }

            auto sceneName = actionScene->name();
            auto actId = act->property(DSC_NAMESPACE::ActionPropertyKey::kActionID).toString();
            if (emptyDisableActions.contains(sceneName, actId)) {
                menu->removeAction(act);
                continue;
            }

            if (sceneNameCurrent.isEmpty())
                sceneNameCurrent = sceneName;

            if (sceneNameCurrent != sceneName) {
                menu->insertSeparator(act);
                sceneNameCurrent = sceneName;
            }
        }
    } else {
        QAction *removeAct = nullptr;
        QAction *copyAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene) {
                // no scene,remove it.
                menu->removeAction(act);
                continue;
            }

            auto sceneName = actionScene->name();
            auto actId = act->property(DSC_NAMESPACE::ActionPropertyKey::kActionID).toString();
            if (selectDisableActions.contains(sceneName, actId))
                menu->removeAction(act);

            const auto &p = act->property(DSC_NAMESPACE::ActionPropertyKey::kActionID);
            if (p == RecentActionID::kRemove) {
                removeAct = act;
            } else if (p == dfmplugin_menu::ActionID::kCopy) {
                copyAct = act;
            }
        }

        actions = menu->actions();

        if (removeAct) {
            actions.removeOne(removeAct);
            menu->addActions(actions);
            menu->insertAction(copyAct, removeAct);
            menu->removeAction(copyAct);
            menu->insertAction(removeAct, copyAct);
        }
    }
}
