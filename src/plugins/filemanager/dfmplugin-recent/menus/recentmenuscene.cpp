// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentmenuscene.h"
#include "private/recentmenuscene_p.h"
#include "utils/recentmanager.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <QMenu>

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";
static constexpr char kFileOperatorMenuSceneName[] = "FileOperatorMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kOpenDirMenuSceneName[] = "OpenDirMenu";
static constexpr char kExtendMenuSceneName[] = "ExtendMenu";
static constexpr char kDConfigMenuFilterSceneName[] = "DConfigMenuFilter";

static constexpr char kSortByActionId[] = "sort-by";
static constexpr char kGroupByActionId[] = "group-by";
static constexpr char kSrtTimeModifiedActionId[] = "sort-by-time-modified";
static constexpr char kSrtTimeCreatedActionId[] = "sort-by-time-created";
static constexpr char kGroupTimeModifiedActionId[] = "group-by-time-modified";
static constexpr char kGroupTimeCreatedActionId[] = "group-by-time-created";

using namespace dfmplugin_recent;
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
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "Menu scene initialization failed - invalid parameters"
                    << "selectFiles isEmpty:" << d->selectFiles.isEmpty()
                    << "focusFile:" << d->focusFile
                    << "currentDir:" << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;
    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmWarning() << "Failed to create focusFileInfo:" << errString;
            return false;
        }
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkspaceMenuSceneName))
            currentScene.append(workspaceScene);
    } else {
        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kSortAndDisplayMenuSceneName))
            currentScene.append(workspaceScene);

        if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenDirMenuSceneName))
            currentScene.append(workspaceScene);
    }

    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigMenuFilterSceneName))
        currentScene.append(filterScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    bool ret = AbstractMenuScene::initialize(params);
    d->disableSubScene(this, kExtendMenuSceneName);

    return ret;
}

bool RecentMenuScene::create(QMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot create menu scene with null parent menu";
        return false;
    }

    if (!d->isEmptyArea) {
        auto actRemove = parent->addAction(d->predicateName[RecentActionID::kRemove]);
        actRemove->setProperty(ActionPropertyKey::kActionID, RecentActionID::kRemove);
        d->predicateAction[RecentActionID::kRemove] = actRemove;

        auto actOpenFileLocation = parent->addAction(d->predicateName[RecentActionID::kOpenFileLocation]);
        actOpenFileLocation->setProperty(ActionPropertyKey::kActionID, RecentActionID::kOpenFileLocation);
        d->predicateAction[RecentActionID::kOpenFileLocation] = actOpenFileLocation;
    } else {
        QAction *actSortByPath = new QAction(d->predicateName[RecentActionID::kSortByPath], parent);
        actSortByPath->setCheckable(true);
        actSortByPath->setProperty(ActionPropertyKey::kActionID, RecentActionID::kSortByPath);
        d->predicateAction[RecentActionID::kSortByPath] = actSortByPath;

        QAction *actSortByLastRead = new QAction(d->predicateName[RecentActionID::kSortByLastRead], parent);
        actSortByLastRead->setCheckable(true);
        actSortByLastRead->setProperty(ActionPropertyKey::kActionID, RecentActionID::kSortByLastRead);
        d->predicateAction[RecentActionID::kSortByLastRead] = actSortByLastRead;

        // group by
        QAction *actGroupByPath = new QAction(d->predicateName[RecentActionID::kGroupByPath], parent);
        actGroupByPath->setCheckable(true);
        actGroupByPath->setProperty(ActionPropertyKey::kActionID, RecentActionID::kGroupByPath);
        d->predicateAction[RecentActionID::kGroupByPath] = actGroupByPath;

        QAction *actGroupByLastRead = new QAction(d->predicateName[RecentActionID::kGroupByLastRead], parent);
        actGroupByLastRead->setCheckable(true);
        actGroupByLastRead->setProperty(ActionPropertyKey::kActionID, RecentActionID::kGroupByLastRead);
        d->predicateAction[RecentActionID::kGroupByLastRead] = actGroupByLastRead;
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
    const QString &actId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actId)) {
        if (actId == RecentActionID::kRemove) {
            RecentHelper::removeRecent(d->selectFiles);
            return true;
        } else if (actId == RecentActionID::kOpenFileLocation) {
            RecentHelper::openFileLocation(d->selectFiles);
            return true;
        } else if (actId == RecentActionID::kSortByPath) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFilePathRole);
            return true;
        } else if (actId == RecentActionID::kSortByLastRead) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileLastReadRole);
            return true;
        } else if (actId == RecentActionID::kGroupByPath) {
            fmDebug() << "Recent: Setting group by path for window:" << d->windowId;
            d->groupByRole(Global::ItemRoles::kItemFilePathRole);
            return true;
        } else if (actId == RecentActionID::kGroupByLastRead) {
            fmDebug() << "Recent: Setting group by last read for window:" << d->windowId;
            d->groupByRole(Global::ItemRoles::kItemFileLastReadRole);
            return true;
        }
        fmWarning() << "Unknown action triggered, actionId:" << actId;
        return false;
    }

    return AbstractMenuScene::triggered(action);
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
    predicateName[RecentActionID::kOpenFileLocation] = tr("Open file location");
    predicateName[RecentActionID::kSortByPath] = tr("Path");
    predicateName[RecentActionID::kSortByLastRead] = tr("Last access");
    predicateName[RecentActionID::kGroupByPath] = tr("Path");
    predicateName[RecentActionID::kGroupByLastRead] = tr("Last access");

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
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (emptyDisableActions.contains(sceneName, actId)) {
                menu->removeAction(act);
                continue;
            }

            if (sceneName == kSortAndDisplayMenuSceneName && actId == kSortByActionId) {
                auto subMenu = act->menu();
                updateSortSubMenu(subMenu);
                continue;
            }

            if (sceneName == kSortAndDisplayMenuSceneName && actId == kGroupByActionId) {
                auto subMenu = act->menu();
                updateGroupSubMenu(subMenu);
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
        QAction *openWithAct = nullptr;
        QAction *openFileLocationAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (selectDisableActions.contains(sceneName, actId))
                menu->removeAction(act);

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == RecentActionID::kRemove) {
                removeAct = act;
            } else if (p == dfmplugin_menu::ActionID::kCopy) {
                copyAct = act;
            } else if (p == dfmplugin_menu::ActionID::kOpenWith) {
                openWithAct = act;
            } else if (p == RecentActionID::kOpenFileLocation) {
                openFileLocationAct = act;
            }
        }

        actions = menu->actions();

        if (removeAct) {
            actions.removeOne(openFileLocationAct);
            actions.removeOne(removeAct);
            menu->addActions(actions);
            menu->insertAction(openWithAct, openFileLocationAct);

            menu->insertAction(copyAct, removeAct);
            menu->removeAction(copyAct);
            menu->insertAction(removeAct, copyAct);
        }
    }
}

void RecentMenuScenePrivate::updateSortSubMenu(QMenu *menu)
{
    QStringList actionsToRemove = { kSrtTimeModifiedActionId, kSrtTimeCreatedActionId };
    QStringList actionsToAdd = { RecentActionID::kSortByLastRead, RecentActionID::kSortByPath };
    QMap<Global::ItemRoles, QString> roleToActionMap = {
        { Global::ItemRoles::kItemFilePathRole, RecentActionID::kSortByPath },
        { Global::ItemRoles::kItemFileLastReadRole, RecentActionID::kSortByLastRead }
    };
    
    updateSubMenuGeneric(menu, actionsToRemove, actionsToAdd, "slot_Model_CurrentSortRole", roleToActionMap);
}

void RecentMenuScenePrivate::updateGroupSubMenu(QMenu *menu)
{
    QStringList actionsToRemove = { kGroupTimeModifiedActionId, kGroupTimeCreatedActionId };
    QStringList actionsToAdd = { RecentActionID::kGroupByLastRead, RecentActionID::kGroupByPath };
    QMap<Global::ItemRoles, QString> roleToActionMap = {
        { Global::ItemRoles::kItemFilePathRole, RecentActionID::kGroupByPath },
        { Global::ItemRoles::kItemFileLastReadRole, RecentActionID::kGroupByLastRead }
    };
    
    updateSubMenuGeneric(menu, actionsToRemove, actionsToAdd, "slot_Model_CurrentGroupRole", roleToActionMap);
}

void RecentMenuScenePrivate::groupByRole(int role)
{
    auto itemRole = static_cast<Global::ItemRoles>(role);
    fmDebug() << "Recent: Grouping by role:" << role;
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetGroup", windowId, itemRole);
}

void RecentMenuScenePrivate::updateSubMenuGeneric(QMenu *menu, 
                                                const QStringList &actionsToRemove,
                                                const QStringList &actionsToAdd,
                                                const QString &currentRoleSlot,
                                                const QMap<Global::ItemRoles, QString> &roleToActionMap)
{
    // Collect all actions to remove
    QList<QAction *> actionsToRemoveList;
    for (auto act : menu->actions()) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        if (actionsToRemove.contains(actId)) {
            actionsToRemoveList.append(act);
        }
    }

    if (!actionsToRemoveList.isEmpty()) {
        // Find insertion point
        QAction *insertBefore = actionsToRemoveList.first();

        // Insert custom actions in reverse order to maintain correct sequence
        for (int i = actionsToAdd.size() - 1; i >= 0; --i) {
            const QString &actionId = actionsToAdd[i];
            if (predicateAction.contains(actionId)) {
                menu->insertAction(insertBefore, predicateAction[actionId]);
            }
        }

        // Remove all matching actions
        for (auto act : actionsToRemoveList) {
            menu->removeAction(act);
        }

        // Update state based on current role
        auto role = dpfSlotChannel->push("dfmplugin_workspace", currentRoleSlot, windowId).value<Global::ItemRoles>();
        if (roleToActionMap.contains(role)) {
            const QString &actionId = roleToActionMap[role];
            if (predicateAction.contains(actionId)) {
                predicateAction[actionId]->setChecked(true);
            }
        }
    }
}

void RecentMenuScenePrivate::disableSubScene(AbstractMenuScene *scene, const QString &sceneName)
{
    for (const auto s : scene->subscene()) {
        if (sceneName == s->name()) {
            scene->removeSubscene(s);
            delete s;
            return;
        } else {
            disableSubScene(s, sceneName);
        }
    }
}
