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

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/dpf.h>

#include <QMenu>

static const char *const kActionIconMenuSceneName = "ActionIconManager";
static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";
static constexpr char kFileOperatorMenuSceneName[] = "FileOperatorMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kPropertyMenuSceneName[] = "PropertyMenu";
static constexpr char kTrashMenuSceneName[] = "TrashMenu";
static const char *const kOemMenuSceneName = "OemMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";

using namespace dfmplugin_trash;
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

    QList<AbstractMenuScene *> currentScene;

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
        if (d->focusFileInfo.isNull()) {
            qDebug() << errString;
            return false;
        }
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kFileOperatorMenuSceneName))
            currentScene.append(workspaceScene);
        if (auto dirScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenDirMenuSceneName))
            currentScene.append(dirScene);
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kClipBoardMenuSceneName))
            currentScene.append(workspaceScene);
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kPropertyMenuSceneName))
            currentScene.append(workspaceScene);
        if (auto oemScene = dfmplugin_menu_util::menuSceneCreateScene(kOemMenuSceneName))
            currentScene.append(oemScene);
    } else {
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kSortAndDisplayMenuSceneName))
            currentScene.append(workspaceScene);

        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kPropertyMenuSceneName))
            currentScene.append(workspaceScene);
    }

    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene(kActionIconMenuSceneName))
        currentScene.append(actionIconManagerScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

bool TrashMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        auto isDisabled = TrashHelper::isEmpty() || !FileUtils::isTrashRootFile(d->currentDir);

        auto act = parent->addAction(d->predicateName[TrashActionId::kRestoreAll]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kRestoreAll);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kRestoreAll] = act;

        act = parent->addAction(d->predicateName[TrashActionId::kEmptyTrash]);
        act->setProperty(ActionPropertyKey::kActionID, TrashActionId::kEmptyTrash);
        act->setEnabled(!isDisabled);
        d->predicateAction[TrashActionId::kEmptyTrash] = act;

        // sort by
        QAction *actSortByPath = new QAction(d->predicateName[TrashActionId::kSourcePath], parent);
        actSortByPath->setCheckable(true);
        actSortByPath->setProperty(ActionPropertyKey::kActionID, TrashActionId::kSourcePath);
        d->predicateAction[TrashActionId::kSourcePath] = actSortByPath;

        QAction *actSortByDeleted = new QAction(d->predicateName[TrashActionId::kTimeDeleted], parent);
        actSortByDeleted->setCheckable(true);
        actSortByDeleted->setProperty(ActionPropertyKey::kActionID, TrashActionId::kTimeDeleted);
        d->predicateAction[TrashActionId::kTimeDeleted] = actSortByDeleted;

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
    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actId)) {
        if (actId == TrashActionId::kRestore) {
            TrashHelper::restoreFromTrashHandle(0, d->selectFiles, AbstractJobHandler::JobFlag::kNoHint);
            return true;
        } else if (actId == TrashActionId::kRestoreAll) {
            TrashHelper::restoreFromTrashHandle(0, { d->currentDir }, AbstractJobHandler::JobFlag::kNoHint);
            return true;
        } else if (actId == TrashActionId::kEmptyTrash) {
            TrashHelper::emptyTrash();
            return true;
        } else if (actId == TrashActionId::kSourcePath) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileOriginalPath);
            return true;
        } else if (actId == TrashActionId::kTimeDeleted) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileDeletionDate);
            return true;
        }
        qWarning() << "action not found, id: " << actId;
        return false;
    } else {
        return AbstractMenuScene::triggered(action);
    }
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
    predicateName[TrashActionId::kSourcePath] = tr("Source path");
    predicateName[TrashActionId::kTimeDeleted] = tr("Time deleted");

    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCut);
    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCopy);
    selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kDelete);
    selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kOpen);
    selectSupportActions.insert(kPropertyMenuSceneName, "property");
    selectSupportActions.insert(kTrashMenuSceneName, TrashActionId::kRestore);
    selectSupportActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
}

void TrashMenuScenePrivate::updateMenu(QMenu *menu)
{
    const QUrl &curDir = this->currentDir;
    auto actions = menu->actions();
    if (isEmptyArea) {
        QString sceneNameCurrent;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (actId == TrashActionId::kRestoreAll
                || actId == TrashActionId::kEmptyTrash)
                act->setEnabled(FileUtils::isTrashRootFile(curDir) && !TrashHelper::isEmpty());

            if (sceneName == "SortAndDisplayMenu" && actId == "sort-by") {
                auto subMenu = act->menu();
                updateSubMenu(subMenu);
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
        //        selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kOpen);
        //        selectSupportActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
        QAction *actionOpen = nullptr;
        QAction *actionOpenNewWind = nullptr;
        QAction *actionRestore = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (!selectSupportActions.contains(sceneName, actId) && sceneName != kOemMenuSceneName)
                menu->removeAction(act);

            if (sceneName == kOemMenuSceneName)
                menu->insertSeparator(act);

            if (sceneName == kPropertyMenuSceneName)
                menu->insertSeparator(act);

            if (sceneName == kFileOperatorMenuSceneName
                && focusFileInfo->isFile()
                && actId == dfmplugin_menu::ActionID::kOpen)
                menu->removeAction(act);

            if (actId == TrashActionId::kRestore
                || actId == dfmplugin_menu::ActionID::kDelete
                || actId == dfmplugin_menu::ActionID::kCut)
                act->setEnabled(FileUtils::isTrashRootFile(curDir));

            if (actId == TrashActionId::kRestore)
                actionRestore = act;
            if (focusFileInfo->isDir()) {
                if (actId == dfmplugin_menu::ActionID::kOpen)
                    actionOpen = act;
                if (actId == dfmplugin_menu::ActionID::kOpenInNewWindow)
                    actionOpenNewWind = act;
            }
        }
        if (actionRestore && actionOpen)
            menu->insertAction(actionRestore, actionOpen);
        if (actionRestore && actionOpenNewWind)
            menu->insertAction(actionRestore, actionOpenNewWind);
        menu->insertSeparator(actionRestore);
    }
}

void TrashMenuScenePrivate::updateSubMenu(QMenu *menu)
{
    auto actions = menu->actions();
    auto iter = std::find_if(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == "sort-by-time-modified";
    });

    if (iter != actions.end()) {
        menu->insertAction(*iter, predicateAction[TrashActionId::kTimeDeleted]);
        menu->insertAction(predicateAction[TrashActionId::kTimeDeleted], predicateAction[TrashActionId::kSourcePath]);
        menu->removeAction(*iter);

        auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
        switch (role) {
        case Global::ItemRoles::kItemFileOriginalPath:
            predicateAction[TrashActionId::kSourcePath]->setChecked(true);
            break;
        case Global::ItemRoles::kItemFileDeletionDate:
            predicateAction[TrashActionId::kTimeDeleted]->setChecked(true);
            break;
        default:
            break;
        }
    }
}
