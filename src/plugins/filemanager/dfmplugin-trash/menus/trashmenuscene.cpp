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
