// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmenuscene.h"
#include "searchmenuscene_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>

#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#include <QProcess>
#include <QMenu>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kExtendMenuSceneName[] = "ExtendMenu";
static constexpr char kDConfigFilterSceneName[] = "DConfigMenuFilter";

static constexpr char kSortByActId[] = "sort-by";
static constexpr char kGroupByActId[] = "group-by";
static constexpr char kDisplayAsActId[] = "display-as";

AbstractMenuScene *SearchMenuCreator::create()
{
    return new SearchMenuScene();
}

SearchMenuScenePrivate::SearchMenuScenePrivate(SearchMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
    emptyWhitelist << kSortByActId
                   << kGroupByActId
                   << kDisplayAsActId
                   << SearchActionId::kSrtPath
                   << dfmplugin_menu::ActionID::kSelectAll;
}

void SearchMenuScenePrivate::createAction(QMenu *menu, const QString &actName, bool isSubAct, bool checkable)
{
    auto actionList = menu->actions();
    auto iter = std::find_if(actionList.begin(), actionList.end(), [&](const QAction *action) {
        const auto &p = action->property(ActionPropertyKey::kActionID);
        return p == actName;
    });

    if (iter == actionList.end()) {
        QAction *tempAction = isSubAct ? new QAction(predicateName.value(actName), menu)
                                       : menu->addAction(predicateName.value(actName));
        predicateAction[actName] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, actName);
        tempAction->setCheckable(checkable);
    }
}

void SearchMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    if (isEmptyArea) {
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actionScene = q->scene(act);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (!emptyWhitelist.contains(actId)) {
                act->setVisible(false);
                continue;
            }

            if (sceneName == kSortAndDisplayMenuSceneName && actId == kSortByActId)
                updateSortMenu(act->menu());
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
            openLocalAct->setVisible(true);
            actions.removeOne(openLocalAct);

            if (actions.size() < 1)
                actions.append(openLocalAct);
            else
                actions.insert(1, openLocalAct);
            menu->addActions(actions);
        }
    }
}

void SearchMenuScenePrivate::updateSortMenu(QMenu *menu)
{
    if (!predicateAction.contains(SearchActionId::kSrtPath))
        return;

    auto actions = menu->actions();
    bool contians = std::any_of(actions.begin(), actions.end(), [](QAction *act) {
        auto actId = act->property(ActionPropertyKey::kActionID).toString();
        return actId == SearchActionId::kSrtPath;
    });
    if (contians)
        return;

    if (actions.size() > 1)
        actions.insert(1, predicateAction[SearchActionId::kSrtPath]);
    else
        actions.append(predicateAction[SearchActionId::kSrtPath]);

    menu->addActions(actions);
    auto role = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_CurrentSortRole", windowId).value<Global::ItemRoles>();
    if (role == Global::ItemRoles::kItemFilePathRole)
        predicateAction[SearchActionId::kSrtPath]->setChecked(true);
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

SearchMenuScene::SearchMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SearchMenuScenePrivate(this))
{
    d->predicateName[SearchActionId::kOpenFileLocation] = tr("Open file location");
    d->predicateName[dfmplugin_menu::ActionID::kSelectAll] = tr("Select all");
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

    if (!d->currentDir.isValid()) {
        fmWarning() << "Invalid current directory for search menu scene";
        return false;
    }

    QVariantHash tmpParams = params;
    QList<AbstractMenuScene *> currentScene;
    const auto &targetUrl = SearchHelper::searchTargetUrl(d->currentDir);

    QString menuScene {};
    if (CustomManager::instance()->isUseNormalMenu(targetUrl.scheme())) {
        menuScene = dpfSlotChannel->push("dfmplugin_workspace", "slot_FindMenuScene", Global::Scheme::kFile).toString();
    } else {
        menuScene = dpfSlotChannel->push("dfmplugin_workspace", "slot_FindMenuScene", targetUrl.scheme()).toString();
    }
    if (auto scene = dfmplugin_menu_util::menuSceneCreateScene(menuScene)) {
        currentScene.append(scene);
        tmpParams[MenuParamKey::kCurrentDir] = targetUrl;
    } else if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkspaceMenuSceneName)) {
        currentScene.append(workspaceScene);
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
    if (!parent) {
        fmWarning() << "Cannot create search menu scene with null parent menu";
        return false;
    }

    // 创建子场景菜单
    AbstractMenuScene::create(parent);

    if (d->isEmptyArea) {
        d->createAction(parent, dfmplugin_menu::ActionID::kSelectAll);

        auto roles = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_ColumnRoles", d->windowId).value<QList<Global::ItemRoles>>();
        if (roles.contains(Global::ItemRoles::kItemFilePathRole))
            d->createAction(parent, SearchActionId::kSrtPath, true, true);
    } else {
        d->createAction(parent, SearchActionId::kOpenFileLocation);
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

        // sort by path
        if (actionId == SearchActionId::kSrtPath) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetSort", d->windowId, Global::ItemRoles::kItemFilePathRole);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
