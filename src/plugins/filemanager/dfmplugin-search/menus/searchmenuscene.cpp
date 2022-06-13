/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#include "searchmenuscene.h"
#include "searchmenuscene_p.h"
#include "utils/searchhelper.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "services/common/menu/menu_defines.h"
#include "services/common/menu/menuservice.h"
#include "services/filemanager/workspace/workspaceservice.h"

#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/schemefactory.h"

#include <DDesktopServices>

#include <QProcess>

DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";

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

SearchMenuScene::SearchMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SearchMenuScenePrivate(this))
{
    d->predicateName[SearchActionId::kOpenFileLocation] = tr("Open file location");
    d->predicateName[dfmplugin_menu::ActionID::kSelectAll] = tr("Select all");
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
        if (auto sortAndDisplayScene = MenuService::service()->createScene(kSortAndDisplayMenuSceneName))
            currentScene.append(sortAndDisplayScene);
    } else {
        const auto &parentUrl = SearchHelper::searchTargetUrl(d->currentDir);
        if (Global::kFile == parentUrl.scheme()) {
            if (auto workspaceScene = MenuService::service()->createScene(kWorkspaceMenuSceneName))
                currentScene.append(workspaceScene);
        } else {
            auto parentSceneName = WorkspaceService::service()->findMenuScene(parentUrl.scheme());
            if (auto scene = MenuService::service()->createScene(parentSceneName))
                currentScene.append(scene);

            const auto &targetUrl = SearchHelper::searchTargetUrl(d->currentDir);
            tmpParams[MenuParamKey::kCurrentDir] = targetUrl;
        }
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    return AbstractMenuScene::initialize(tmpParams);
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

    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(dfmplugin_menu::ActionID::kSelectAll));
        d->predicateAction[dfmplugin_menu::ActionID::kSelectAll] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(dfmplugin_menu::ActionID::kSelectAll));
    } else {
        QAction *tempAction = parent->addAction(d->predicateName.value(SearchActionId::kOpenFileLocation));
        d->predicateAction[SearchActionId::kOpenFileLocation] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(SearchActionId::kOpenFileLocation));
    }

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
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
                auto info = InfoFactory::create<AbstractFileInfo>(file);
                d->openFileLocation(info->absoluteFilePath());
            }

            return true;
        }

        // select all
        if (actionId == dfmplugin_menu::ActionID::kSelectAll) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_SelectAll", d->windowId);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
