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
#include "workspacemenuscene.h"
#include "workspacemenuscene_p.h"

#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "utils/fileoperatorhelper.h"

#include "dfm-base/dfm_global_defines.h"

DPWORKSPACE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

static const char *const kNewCreateMenuSceneName = "NewCreateMenu";
static const char *const kClipBoardMenuSceneName = "ClipBoardMenu";
static const char *const kOpenWithMenuSceneName = "OpenWithMenu";
static const char *const kOpenFileMenuSceneName = "OpenFileMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";

AbstractMenuScene *WorkspaceMenuCreator::create()
{
    return new WorkspaceMenuScene();
}

WorkspaceMenuScenePrivate::WorkspaceMenuScenePrivate(WorkspaceMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
    menuServer = MenuService::service();
}

void WorkspaceMenuScenePrivate::sortMenuAction(QMenu *menu, const QStringList &sortRule)
{
    auto actions = menu->actions();
    qSort(actions.begin(), actions.end(), [&sortRule](QAction *act1, QAction *act2) {
        const auto &property1 = act1->property(ActionPropertyKey::kActionID).toString();
        const auto &property2 = act2->property(ActionPropertyKey::kActionID).toString();

        auto index1 = sortRule.indexOf(property1);
        if (index1 == -1)
            return false;

        auto index2 = sortRule.indexOf(property2);
        if (index2 == -1)
            return true;

        return index1 < index2;
    });

    // insert separator
    int index = sortRule.indexOf(ActionID::kSeparator);
    while (index != -1) {
        QAction *separatorAct = new QAction(menu);
        separatorAct->setSeparator(true);
        actions.insert(index, separatorAct);
        index = sortRule.indexOf(ActionID::kSeparator, index + 1);
    }

    menu->addActions(actions);
}

WorkspaceMenuScene::WorkspaceMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new WorkspaceMenuScenePrivate(this))
{
}

QString WorkspaceMenuScene::name() const
{
    return WorkspaceMenuCreator::name();
}

bool WorkspaceMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    if (d->currentDir.isEmpty())
        return false;

    QList<AbstractMenuScene *> currentScene;

    // file operation
    if (auto operationScene = d->menuServer->createScene(kClipBoardMenuSceneName))
        currentScene.append(operationScene);

    // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
    if (auto dirScene = d->menuServer->createScene(kOpenDirMenuSceneName))
        currentScene.append(dirScene);

    if (d->isEmptyArea) {
        // new (new doc, new dir)
        if (auto newCreateScene = d->menuServer->createScene(kNewCreateMenuSceneName))
            currentScene.append(newCreateScene);
    } else {
        // open with
        if (auto openWithScene = d->menuServer->createScene(kOpenWithMenuSceneName))
            currentScene.append(openWithScene);

        // file (rename)
        if (auto fileScene = d->menuServer->createScene(kOpenFileMenuSceneName))
            currentScene.append(fileScene);

        if (auto sendToScene = d->menuServer->createScene(kSendToMenuSceneName))
            currentScene.append(sendToScene);
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    subScene = currentScene;

    // 初始化所有子场景
    AbstractMenuScene::initialize(params);

    return true;
}

AbstractMenuScene *WorkspaceMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<WorkspaceMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool WorkspaceMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    d->view = qobject_cast<FileView *>(parent->parent());
    Q_ASSERT(d->view);

    if (!d->isEmptyArea) {
        this->createNormalMenu(parent);
    }

    // 创建子场景菜单
    AbstractMenuScene::create(parent);
    return true;
}

void WorkspaceMenuScene::updateState(QMenu *parent)
{
    if (d->isEmptyArea) {
        d->sortMenuAction(parent, d->emptyMenuActionRule());
    }
    // todo: sort item (liuzhangjian)
    AbstractMenuScene::updateState(parent);
}

bool WorkspaceMenuScene::triggered(QAction *action)
{
    if (d->isEmptyArea)
        return emptyMenuTriggered(action);

    return normalMenuTriggered(action);
}

void WorkspaceMenuScene::createNormalMenu(QMenu *parent)
{
    // todo (liuzhangjian)
    Q_UNUSED(parent);
}

bool WorkspaceMenuScene::emptyMenuTriggered(QAction *action)
{
    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();

    auto actionScene = scene(action);
    if (!actionScene) {
        qWarning() << actionId << " doesn't belong to any scene.";
        return false;
    }

    const QString &sceneName = actionScene->name();
    // OpenDirMenu scene
    if (sceneName == kOpenDirMenuSceneName) {
        // select all
        if (actionId == dfmplugin_menu::ActionID::kSelectAll) {
            d->view->selectAll();
            return true;
        }
    }

    // ClipBoardMenu scene
    if (sceneName == kClipBoardMenuSceneName) {
        // paste
        if (actionId == dfmplugin_menu::ActionID::kPaste) {
            FileOperatorHelperIns->pasteFiles(d->view);
            return true;
        }
    }

    // NewCreateMenu scene
    if (sceneName == kNewCreateMenuSceneName) {
        // new folder
        if (actionId == dfmplugin_menu::ActionID::kNewFolder) {
            FileOperatorHelperIns->touchFolder(d->view);
            return true;
        }

        // new office text
        if (actionId == dfmplugin_menu::ActionID::kNewOfficeText) {
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord);
            return true;
        }

        // new spreadsheets
        if (actionId == dfmplugin_menu::ActionID::kNewSpreadsheets) {
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeExcel);
            return true;
        }

        // new presentation
        if (actionId == dfmplugin_menu::ActionID::kNewPresentation) {
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypePowerpoint);
            return true;
        }

        // new plain text
        if (actionId == dfmplugin_menu::ActionID::kNewPlainText) {
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}

bool WorkspaceMenuScene::normalMenuTriggered(QAction *action)
{
    // todo (liuzhangjian)
    return AbstractMenuScene::triggered(action);
}
