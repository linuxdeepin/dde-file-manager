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

#include "trashmenuscene.h"
#include "private/trashmenuscene_p.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"

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

static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";
static constexpr char kFileOperatorMenuSceneName[] = "FileOperatorMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kPropertyMenuSceneName[] = "Property";
static constexpr char kTrashMenuSceneName[] = "TrashMenu";

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractMenuScene *TrashMenuCreator::create()
{
    return new TrashMenuScene();
}

TrashMenuScene::TrashMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new TrashMenuScenePrivate(this))
{
}

TrashMenuScene::~TrashMenuScene()
{
}

QString TrashMenuScene::name() const
{
    return TrashMenuCreator::name();
}

bool TrashMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE

    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();

    if (!d->isEmptyArea) {
        if (d->selectFiles.isEmpty() || !d->focusFile.isValid() || !d->currentDir.isValid()) {
            qDebug() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
            return false;
        }

        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
        QList<AbstractMenuScene *> currentScene;
        if (auto workspaceScene = MenuService::service()->createScene(kFileOperatorMenuSceneName))
            currentScene.append(workspaceScene);
        if (auto workspaceScene = MenuService::service()->createScene(kClipBoardMenuSceneName))
            currentScene.append(workspaceScene);
        if (auto workspaceScene = MenuService::service()->createScene(kPropertyMenuSceneName))
            currentScene.append(workspaceScene);

        // the scene added by binding must be initializeed after 'defalut scene'.
        currentScene.append(subScene);
        setSubscene(currentScene);
    } else {
        QList<AbstractMenuScene *> currentScene;
        if (auto workspaceScene = MenuService::service()->createScene(kSortAndDisplayMenuSceneName))
            currentScene.append(workspaceScene);

        if (auto workspaceScene = MenuService::service()->createScene(kPropertyMenuSceneName))
            currentScene.append(workspaceScene);

        // the scene added by binding must be initializeed after 'defalut scene'.
        currentScene.append(subScene);
        setSubscene(currentScene);
    }

    return AbstractMenuScene::initialize(params);
    ;
}

bool TrashMenuScene::create(QMenu *parent)
{
    DSC_USE_NAMESPACE

    if (d->isEmptyArea) {
        auto isDisabled = TrashHelper::isEmpty() || !UniversalUtils::urlEquals(d->currentDir, TrashHelper::rootUrl());

        auto act = parent->addAction(d->predicateName[TrashActionId::kRestoreAll]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kRestoreAll);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kRestoreAll] = act;

        act = parent->addAction(d->predicateName[TrashActionId::kEmptyTrash]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kEmptyTrash);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kEmptyTrash] = act;
        parent->addSeparator();
    } else {
        auto act = parent->addAction(d->predicateName[TrashActionId::kRestore]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kRestore);
        d->predicateAction[TrashActionId::kRestore] = act;
    }
    return AbstractMenuScene::create(parent);
}

void TrashMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
    d->updateMenu(parent);
}

bool TrashMenuScene::triggered(QAction *action)
{
    DSC_USE_NAMESPACE
    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actId))
        return false;

    if (actId == TrashActionId::kRestore) {
        TrashFileHelper::restoreFromTrashHandle(0, d->selectFiles, AbstractJobHandler::JobFlag::kRevocation);
        return true;
    } else if (actId == TrashActionId::kRestoreAll) {
        TrashFileHelper::restoreFromTrashHandle(0, { d->currentDir }, AbstractJobHandler::JobFlag::kRevocation);
        return true;
    } else if (actId == TrashActionId::kEmptyTrash) {
        TrashHelper::emptyTrash();
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *TrashMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<TrashMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

TrashMenuScenePrivate::TrashMenuScenePrivate(TrashMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
    predicateName[TrashActionId::kRestore] = tr("Restore");
    predicateName[TrashActionId::kRestoreAll] = tr("Restore all");
    predicateName[TrashActionId::kEmptyTrash] = tr("Empty trash");

    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCut);
    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCopy);
    selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kDelete);
    selectSupportActions.insert(kPropertyMenuSceneName, dfmplugin_menu::ActionID::kProperty);
    selectSupportActions.insert(kTrashMenuSceneName, TrashActionId::kRestore);
}

void TrashMenuScenePrivate::updateMenu(QMenu *menu)
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

            if (sceneNameCurrent.isEmpty())
                sceneNameCurrent = sceneName;

            if (sceneNameCurrent != sceneName) {
                menu->insertSeparator(act);
                sceneNameCurrent = sceneName;
            }
        }
    } else {
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
            if (!selectSupportActions.contains(sceneName, actId))
                menu->removeAction(act);

            if (sceneName == kPropertyMenuSceneName)
                menu->insertSeparator(act);
        }
    }
}
