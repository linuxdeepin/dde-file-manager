// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashmenuscene.h"
#include "private/trashmenuscene_p.h"
#include "utils/trashhelper.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>

static constexpr char kActionIconMenuSceneName[] = "ActionIconManager";
static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";
static constexpr char kFileOperatorMenuSceneName[] = "FileOperatorMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kPropertyMenuSceneName[] = "PropertyMenu";
static constexpr char kTrashMenuSceneName[] = "TrashMenu";
static constexpr char kOemMenuSceneName[] = "OemMenu";
static constexpr char kOpenDirMenuSceneName[] = "OpenDirMenu";
static constexpr char kDConfigMenuFilterSceneName[] = "DConfigMenuFilter";

static constexpr char kSortByActionId[] = "sort-by";
static constexpr char kGroupByActionId[] = "group-by";
static constexpr char kSrtTimeModifiedActionId[] = "sort-by-time-modified";
static constexpr char kSrtTimeCreatedActionId[] = "sort-by-time-created";
static constexpr char kGroupTimeModifiedActionId[] = "group-by-time-modified";
static constexpr char kGroupTimeCreatedActionId[] = "group-by-time-created";

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
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
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

    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigMenuFilterSceneName))
        currentScene.append(filterScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

bool TrashMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        auto isDisabled = FileUtils::trashIsEmpty() || !FileUtils::isTrashRootFile(d->currentDir);

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

        // group by
        QAction *actGroupByPath = new QAction(d->predicateName[TrashActionId::kGroupBySourcePath], parent);
        actGroupByPath->setCheckable(true);
        actGroupByPath->setProperty(ActionPropertyKey::kActionID, TrashActionId::kGroupBySourcePath);
        d->predicateAction[TrashActionId::kGroupBySourcePath] = actGroupByPath;

        QAction *actGroupByDeleted = new QAction(d->predicateName[TrashActionId::kGroupByTimeDeleted], parent);
        actGroupByDeleted->setCheckable(true);
        actGroupByDeleted->setProperty(ActionPropertyKey::kActionID, TrashActionId::kGroupByTimeDeleted);
        d->predicateAction[TrashActionId::kGroupByTimeDeleted] = actGroupByDeleted;

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
            fmDebug() << "Trash: Restoring" << d->selectFiles.size() << "files from trash";
            TrashHelper::restoreFromTrashHandle(0, d->selectFiles, AbstractJobHandler::JobFlag::kNoHint);
            return true;
        } else if (actId == TrashActionId::kRestoreAll) {
            fmDebug() << "Trash: Restoring all files from trash directory:" << d->currentDir;
            TrashHelper::restoreFromTrashHandle(0, { d->currentDir }, AbstractJobHandler::JobFlag::kNoHint);
            return true;
        } else if (actId == TrashActionId::kEmptyTrash) {
            fmDebug() << "Trash: Emptying trash";
            TrashHelper::emptyTrash();
            return true;
        } else if (actId == TrashActionId::kSourcePath) {
            fmDebug() << "Trash: Setting sort by source path for window:" << d->windowId;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileOriginalPath);
            return true;
        } else if (actId == TrashActionId::kTimeDeleted) {
            fmDebug() << "Trash: Setting sort by deletion time for window:" << d->windowId;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileDeletionDate);
            return true;
        } else if (actId == TrashActionId::kGroupBySourcePath) {
            fmDebug() << "Trash: Setting group by source path for window:" << d->windowId;
            d->groupByRole(Global::ItemRoles::kItemFileOriginalPath);
            return true;
        } else if (actId == TrashActionId::kGroupByTimeDeleted) {
            fmDebug() << "Trash: Setting group by deletion time for window:" << d->windowId;
            d->groupByRole(Global::ItemRoles::kItemFileDeletionDate);
            return true;
        }
        return false;
    } else if (auto s = scene(action)) {
        if (s->name() == kOpenDirMenuSceneName
            && actId == dfmplugin_menu::ActionID::kReverseSelect) {
            dpfSlotChannel->push("dfmplugin_workspace",
                                 "slot_View_ReverseSelect", d->windowId);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *TrashMenuScene::scene(QAction *action) const
{
    if (action == nullptr) {
        fmDebug() << "Trash: Scene requested for null action";
        return nullptr;
    }

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
    predicateName[TrashActionId::kGroupBySourcePath] = tr("Source path");
    predicateName[TrashActionId::kGroupByTimeDeleted] = tr("Time deleted");

    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCut);
    selectSupportActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kCopy);
    selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kDelete);
    selectSupportActions.insert(kFileOperatorMenuSceneName, dfmplugin_menu::ActionID::kOpen);
    selectSupportActions.insert(kPropertyMenuSceneName, "property");
    selectSupportActions.insert(kTrashMenuSceneName, TrashActionId::kRestore);
    selectSupportActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
    selectSupportActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kReverseSelect);
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
                act->setEnabled(FileUtils::isTrashRootFile(curDir) && !FileUtils::trashIsEmpty());

            if (sceneName == "SortAndDisplayMenu" && actId == kSortByActionId) {
                auto subMenu = act->menu();
                updateSortSubMenu(subMenu);
                continue;
            }

            if (sceneName == "SortAndDisplayMenu" && actId == kGroupByActionId) {
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
                && focusFileInfo->isAttributes(OptInfoType::kIsFile)
                && actId == dfmplugin_menu::ActionID::kOpen)
                menu->removeAction(act);

            if (actId == TrashActionId::kRestore
                || actId == dfmplugin_menu::ActionID::kDelete
                || actId == dfmplugin_menu::ActionID::kCut) {
                // 这还应该判断当前文件的父目录
                auto fileurl = focusFileInfo->urlOf(UrlInfoType::kParentUrl);
                act->setEnabled(FileUtils::isTrashRootFile(fileurl));
            }

            if (actId == TrashActionId::kRestore)
                actionRestore = act;
            if (focusFileInfo->isAttributes(OptInfoType::kIsDir)) {
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

void TrashMenuScenePrivate::updateSortSubMenu(QMenu *menu)
{
    QStringList actionsToRemove = { kSrtTimeModifiedActionId, kSrtTimeCreatedActionId };
    QStringList actionsToAdd = { TrashActionId::kTimeDeleted, TrashActionId::kSourcePath };
    QMap<Global::ItemRoles, QString> roleToActionMap = {
        { Global::ItemRoles::kItemFileOriginalPath, TrashActionId::kSourcePath },
        { Global::ItemRoles::kItemFileDeletionDate, TrashActionId::kTimeDeleted }
    };
    
    updateSubMenuGeneric(menu, actionsToRemove, actionsToAdd, "slot_Model_CurrentSortRole", roleToActionMap);
}

void TrashMenuScenePrivate::updateGroupSubMenu(QMenu *menu)
{
    QStringList actionsToRemove = { kGroupTimeModifiedActionId, kGroupTimeCreatedActionId };
    QStringList actionsToAdd = { TrashActionId::kGroupByTimeDeleted, TrashActionId::kGroupBySourcePath };
    QMap<Global::ItemRoles, QString> roleToActionMap = {
        { Global::ItemRoles::kItemFileOriginalPath, TrashActionId::kGroupBySourcePath },
        { Global::ItemRoles::kItemFileDeletionDate, TrashActionId::kGroupByTimeDeleted }
    };
    
    updateSubMenuGeneric(menu, actionsToRemove, actionsToAdd, "slot_Model_CurrentGroupRole", roleToActionMap);
}

void TrashMenuScenePrivate::updateSubMenuGeneric(QMenu *menu, 
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

void TrashMenuScenePrivate::groupByRole(int role)
{
    auto itemRole = static_cast<Global::ItemRoles>(role);
    fmDebug() << "Trash: Grouping by role:" << role;
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetGroup", windowId, itemRole);
}
