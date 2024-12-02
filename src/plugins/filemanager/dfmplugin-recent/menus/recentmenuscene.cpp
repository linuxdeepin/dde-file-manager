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
static constexpr char kSrtTimeModifiedActionId[] = "sort-by-time-modified";

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
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QList<AbstractMenuScene *> currentScene;
    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmWarning() << "focusFileInfo isNull :" << errString;
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
    if (!parent)
        return false;
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
        }
        fmWarning() << "action not found, id: " << actId;
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

void RecentMenuScenePrivate::updateSubMenu(QMenu *menu)
{
    auto actions = menu->actions();
    auto iter = std::find_if(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == kSrtTimeModifiedActionId;
    });

    if (iter != actions.end()) {
        menu->insertAction(*iter, predicateAction[RecentActionID::kSortByLastRead]);
        menu->insertAction(predicateAction[RecentActionID::kSortByLastRead], predicateAction[RecentActionID::kSortByPath]);
        menu->removeAction(*iter);

        auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
        switch (role) {
        case Global::ItemRoles::kItemFilePathRole:
            predicateAction[RecentActionID::kSortByPath]->setChecked(true);
            break;
        case Global::ItemRoles::kItemFileLastReadRole:
            predicateAction[RecentActionID::kSortByLastRead]->setChecked(true);
            break;
        default:
            break;
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
