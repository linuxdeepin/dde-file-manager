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

#include "plugins/common/dfmplugin-menu/menuScene/action_defines.h"
#include "services/common/menu/menu_defines.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/schemefactory.h"

#include <DDesktopServices>

#include <QProcess>

DWIDGET_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kOpenDirMenuSceneName[] = "OpenDirMenu";
static constexpr char kNewCreateMenuSceneName[] = "NewCreateMenu";
static constexpr char kClipBoardMenuSceneName[] = "ClipBoardMenu";

AbstractMenuScene *SearchMenuCreator::create()
{
    return new SearchMenuScene();
}

SearchMenuScenePrivate::SearchMenuScenePrivate(SearchMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenAsAdmin);
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInTerminal);
    emptyDisableActions.insert(kNewCreateMenuSceneName, dfmplugin_menu::ActionID::kNewFolder);
    emptyDisableActions.insert(kNewCreateMenuSceneName, dfmplugin_menu::ActionID::kNewDoc);
    emptyDisableActions.insert(kClipBoardMenuSceneName, dfmplugin_menu::ActionID::kPaste);
}

void SearchMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    if (isEmptyArea) {
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
            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (emptyDisableActions.contains(sceneName, actId))
                menu->removeAction(act);
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
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    if (!d->currentDir.isValid())
        return false;

    if (!d->isEmptyArea) {
        if (d->selectFiles.isEmpty())
            return false;

        // convert to real file url
        d->focusFile = SearchHelper::searchedFileUrl(d->focusFile);
        for (auto &url : d->selectFiles)
            url = SearchHelper::searchedFileUrl(url);
    }

    QList<AbstractMenuScene *> currentScene;
    if (auto workspaceScene = MenuService::service()->createScene(kWorkspaceMenuSceneName))
        currentScene.append(workspaceScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    QVariantHash tmpParams = params;
    tmpParams[MenuParamKey::kFocusFile] = d->focusFile;
    tmpParams[MenuParamKey::kSelectFiles] = QVariant::fromValue(d->selectFiles);
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

    if (!d->isEmptyArea) {
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
    if (actionId == SearchActionId::kOpenFileLocation) {
        for (const auto &file : d->selectFiles) {
            auto info = InfoFactory::create<AbstractFileInfo>(file);
            d->openFileLocation(info->absoluteFilePath());
        }

        return true;
    }

    return AbstractMenuScene::triggered(action);
}
