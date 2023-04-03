// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmenuscene.h"
#include "searchmenuscene_p.h"
#include "utils/searchhelper.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>

#include <DDesktopServices>

#include <QProcess>
#include <QMenu>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kExtendMenuSceneName[] = "ExtendMenu";
static constexpr char kDConfigFilterSceneName[] = "DConfigMenuFilter";

AbstractMenuScene *SearchMenuCreator::create()
{
    return new SearchMenuScene();
}

SearchMenuScenePrivate::SearchMenuScenePrivate(SearchMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

void SearchMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    if (isEmptyArea) {
        QAction *selAllAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();

            if (sceneName == "SortAndDisplayMenu" && actId == "sort-by") {
                auto subMenu = act->menu();
                updateSubMenu(subMenu);
                continue;
            }

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == dfmplugin_menu::ActionID::kSelectAll) {
                selAllAct = act;
                break;
            }
        }

        if (selAllAct) {
            actions.removeOne(selAllAct);
            actions.append(selAllAct);
            menu->addActions(actions);
            menu->insertSeparator(selAllAct);
        }

    } else {
        QAction *openLocalAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == SearchActionId::kOpenFileLocation) {
                openLocalAct = act;
                break;
            }
        }

        // insert 'OpenFileLocation' action
        if (openLocalAct) {
            actions.removeOne(openLocalAct);
            actions.insert(1, openLocalAct);
            menu->addActions(actions);
        }
    }
}

bool SearchMenuScenePrivate::openFileLocation(const QString &path)
{
    // why? because 'DDesktopServices::showFileItem(realUrl(event->url()))' will call session bus 'org.freedesktop.FileManager1'
    // but cannot find session bus when user is root!
    if (SysInfoUtils::isRootUser()) {
        QStringList urls { path };
        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    return DDesktopServices::showFileItem(path);
}

void SearchMenuScenePrivate::disableSubScene(AbstractMenuScene *scene, const QString &sceneName)
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

void SearchMenuScenePrivate::updateSubMenu(QMenu *menu)
{
    const auto &targetUrl = SearchHelper::searchTargetUrl(currentDir);
    if (targetUrl.scheme() == Global::Scheme::kTrash)
        return updateSubMenuTrash(menu);

    if (targetUrl.scheme() == Global::Scheme::kRecent)
        return updateSubMenuRecent(menu);

    return updateSubMenuNormal(menu);
}

void SearchMenuScenePrivate::updateSubMenuNormal(QMenu *menu)
{
    auto actions = menu->actions();
    auto iter = std::find_if(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == "sort-by-name";
    });

    if (iter != actions.end()) {
        menu->insertAction(*iter, predicateAction[SearchActionId::kSrtPath]);
        auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
        if (role == Global::ItemRoles::kItemFilePathRole)
            predicateAction[SearchActionId::kSrtPath]->setChecked(true);
    }
}

void SearchMenuScenePrivate::updateSubMenuTrash(QMenu *menu)
{
    auto actions = menu->actions();
    auto iter = std::find_if(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == "sort-by-time-modified";
    });

    if (iter != actions.end()) {
        menu->insertAction(*iter, predicateAction[SearchActionId::kTimeDeleted]);
        menu->insertAction(predicateAction[SearchActionId::kTimeDeleted], predicateAction[SearchActionId::kSourcePath]);
        menu->removeAction(*iter);

        auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
        switch (role) {
        case Global::ItemRoles::kItemFileOriginalPath:
            predicateAction[SearchActionId::kSourcePath]->setChecked(true);
            break;
        case Global::ItemRoles::kItemFileDeletionDate:
            predicateAction[SearchActionId::kTimeDeleted]->setChecked(true);
            break;
        default:
            break;
        }
    }
}

void SearchMenuScenePrivate::updateSubMenuRecent(QMenu *menu)
{
    auto actions = menu->actions();
    auto iter = std::find_if(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == "sort-by-time-modified";
    });

    if (iter != actions.end()) {
        menu->insertAction(*iter, predicateAction[SearchActionId::kSortByLastRead]);
        menu->insertAction(predicateAction[SearchActionId::kSortByLastRead], predicateAction[SearchActionId::kSrtPath]);
        menu->removeAction(*iter);

        auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
        switch (role) {
        case Global::ItemRoles::kItemFilePathRole:
            predicateAction[SearchActionId::kSrtPath]->setChecked(true);
            break;
        case Global::ItemRoles::kItemFileLastReadRole:
            predicateAction[SearchActionId::kSortByLastRead]->setChecked(true);
            break;
        default:
            break;
        }
    }
}

SearchMenuScene::SearchMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SearchMenuScenePrivate(this))
{
    d->predicateName[SearchActionId::kOpenFileLocation] = tr("Open file location");
    d->predicateName[dfmplugin_menu::ActionID::kSelectAll] = tr("Select all");
    d->predicateName[SearchActionId::kSourcePath] = tr("Source path");
    d->predicateName[SearchActionId::kTimeDeleted] = tr("Time deleted");
    d->predicateName[SearchActionId::kSortByLastRead] = tr("Time read");
    d->predicateName[SearchActionId::kSrtPath] = tr("Path");
}

SearchMenuScene::~SearchMenuScene()
{
}

QString SearchMenuScene::name() const
{
    return SearchMenuCreator::name();
}

bool SearchMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->currentDir.isValid())
        return false;

    QVariantHash tmpParams = params;
    QList<AbstractMenuScene *> currentScene;
    if (d->isEmptyArea) {
        if (auto sortAndDisplayScene = dfmplugin_menu_util::menuSceneCreateScene(kSortAndDisplayMenuSceneName))
            currentScene.append(sortAndDisplayScene);
    } else {
        const auto &targetUrl = SearchHelper::searchTargetUrl(d->currentDir);
        if (targetUrl.scheme() == Global::Scheme::kTrash || targetUrl.scheme() == Global::Scheme::kRecent) {
            auto parentSceneName = dpfSlotChannel->push("dfmplugin_workspace", "slot_FindMenuScene", targetUrl.scheme()).toString();
            if (auto scene = dfmplugin_menu_util::menuSceneCreateScene(parentSceneName))
                currentScene.append(scene);

            tmpParams[MenuParamKey::kCurrentDir] = targetUrl;
        } else {
            if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkspaceMenuSceneName))
                currentScene.append(workspaceScene);
        }
    }

    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigFilterSceneName))
        currentScene.append(filterScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    bool ret = AbstractMenuScene::initialize(params);
    d->disableSubScene(this, kExtendMenuSceneName);
    return ret;
}

AbstractMenuScene *SearchMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<SearchMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool SearchMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    // 创建子场景菜单
    AbstractMenuScene::create(parent);

    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(dfmplugin_menu::ActionID::kSelectAll));
        d->predicateAction[dfmplugin_menu::ActionID::kSelectAll] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(dfmplugin_menu::ActionID::kSelectAll));

        const auto &targetUrl = SearchHelper::searchTargetUrl(d->currentDir);
        if (targetUrl.scheme() == Global::Scheme::kTrash) {
            // sort by
            QAction *actSortByPath = new QAction(d->predicateName[SearchActionId::kSourcePath], parent);
            actSortByPath->setCheckable(true);
            actSortByPath->setProperty(ActionPropertyKey::kActionID, SearchActionId::kSourcePath);
            d->predicateAction[SearchActionId::kSourcePath] = actSortByPath;

            QAction *actSortByDeleted = new QAction(d->predicateName[SearchActionId::kTimeDeleted], parent);
            actSortByDeleted->setCheckable(true);
            actSortByDeleted->setProperty(ActionPropertyKey::kActionID, SearchActionId::kTimeDeleted);
            d->predicateAction[SearchActionId::kTimeDeleted] = actSortByDeleted;
        } else if (targetUrl.scheme() == Global::Scheme::kRecent) {
            QAction *actSortByPath = new QAction(d->predicateName[SearchActionId::kSrtPath], parent);
            actSortByPath->setCheckable(true);
            actSortByPath->setProperty(ActionPropertyKey::kActionID, SearchActionId::kSrtPath);
            d->predicateAction[SearchActionId::kSrtPath] = actSortByPath;

            QAction *actSortByLastRead = new QAction(d->predicateName[SearchActionId::kSortByLastRead], parent);
            actSortByLastRead->setCheckable(true);
            actSortByLastRead->setProperty(ActionPropertyKey::kActionID, SearchActionId::kSortByLastRead);
            d->predicateAction[SearchActionId::kSortByLastRead] = actSortByLastRead;
        } else {
            QAction *actSortByPath = new QAction(d->predicateName[SearchActionId::kSrtPath], parent);
            actSortByPath->setCheckable(true);
            actSortByPath->setProperty(ActionPropertyKey::kActionID, SearchActionId::kSrtPath);
            d->predicateAction[SearchActionId::kSrtPath] = actSortByPath;
        }
    } else {
        auto actionList = parent->actions();
        auto iter = std::find_if(actionList.begin(), actionList.end(), [](const QAction *action) {
            const auto &p = action->property(ActionPropertyKey::kActionID);
            return (p == SearchActionId::kOpenFileLocation);
        });

        if (iter == actionList.end()) {
            QAction *tempAction = parent->addAction(d->predicateName.value(SearchActionId::kOpenFileLocation));
            d->predicateAction[SearchActionId::kOpenFileLocation] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(SearchActionId::kOpenFileLocation));
        }
    }

    return true;
}

void SearchMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
    d->updateMenu(parent);
}

bool SearchMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actionId)) {
        // open file location
        if (actionId == SearchActionId::kOpenFileLocation) {
            for (const auto &file : d->selectFiles) {
                auto info = InfoFactory::create<FileInfo>(file);
                d->openFileLocation(info->pathOf(PathInfoType::kAbsoluteFilePath));
            }

            return true;
        }

        // select all
        if (actionId == dfmplugin_menu::ActionID::kSelectAll) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SelectAll", d->windowId);
            return true;
        }
        // sort by source path
        if (actionId == SearchActionId::kSourcePath) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileOriginalPath);
            return true;
        }
        // sort by deleted time
        if (actionId == SearchActionId::kTimeDeleted) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileDeletionDate);
            return true;
        }
        // sort by path
        if (actionId == SearchActionId::kSrtPath) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFilePathRole);
            return true;
        }
        // sort by lastread
        if (actionId == SearchActionId::kSortByLastRead) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFileLastReadRole);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
