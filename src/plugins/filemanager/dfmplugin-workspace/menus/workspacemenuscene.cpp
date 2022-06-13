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
#include "events/workspaceeventcaller.h"

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
static const char *const kFileOperatorMenuSceneName = "FileOperatorMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";
static const char *const kExtendMenuSceneName = "ExtendMenu";
static const char *const kOemMenuSceneName = "OemMenu";

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
    // sort
    qSort(actions.begin(), actions.end(), [&sortRule](QAction *act1, QAction *act2) {
        const auto &property1 = act1->property(ActionPropertyKey::kActionID).toString();
        auto index1 = sortRule.indexOf(property1);
        if (index1 == -1)
            return false;

        const auto &property2 = act2->property(ActionPropertyKey::kActionID).toString();
        auto index2 = sortRule.indexOf(property2);
        if (index2 == -1)
            return true;

        return index1 < index2;
    });

    // insert separator func
    std::function<void(int)> insertSeparator;
    insertSeparator = [&](int index) {
        if (index >= sortRule.size() || sortRule[index] == dfmplugin_menu::ActionID::kSeparator)
            return;

        auto rule = sortRule[index];
        auto iter = std::find_if(actions.begin(), actions.end(), [&rule](const QAction *act) {
            auto p = act->property(ActionPropertyKey::kActionID);
            return p == rule;
        });

        if (iter != actions.end()) {
            QAction *separatorAct = new QAction(menu);
            separatorAct->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kSeparator);
            separatorAct->setSeparator(true);
            actions.insert(iter, separatorAct);
        } else {
            insertSeparator(++index);
        }
    };

    // insert separator
    int index = sortRule.indexOf(dfmplugin_menu::ActionID::kSeparator);
    while (index != -1) {
        if (++index >= sortRule.size())
            break;

        insertSeparator(index);
        index = sortRule.indexOf(dfmplugin_menu::ActionID::kSeparator, index);
    }

    menu->addActions(actions);
}

WorkspaceMenuScene::WorkspaceMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new WorkspaceMenuScenePrivate(this))
{
    d->predicateName[ActionID::kRefresh] = tr("Refresh");
}

QString WorkspaceMenuScene::name() const
{
    return WorkspaceMenuCreator::name();
}

bool WorkspaceMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dpfSlotChannel->push("dfmplugin_menu", "slot_PerfectMenuParams", params).value<QVariantHash>();
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();

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

        // file (open, rename, delete)
        if (auto fileScene = d->menuServer->createScene(kFileOperatorMenuSceneName))
            currentScene.append(fileScene);

        if (auto sendToScene = d->menuServer->createScene(kSendToMenuSceneName))
            currentScene.append(sendToScene);
    }

    if (!d->isDDEDesktopFileIncluded) {
        // oem menu
        if (auto oemScene = d->menuServer->createScene(kOemMenuSceneName))
            currentScene.append(oemScene);

        // extend menu.must last
        if (auto extendScene = d->menuServer->createScene(kExtendMenuSceneName))
            currentScene.append(extendScene);
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    return AbstractMenuScene::initialize(params);
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

    if (d->isEmptyArea) {
        if (d->isRefreshOn()) {
            auto tempAction = parent->addAction(d->predicateName.value(ActionID::kRefresh));
            d->predicateAction[ActionID::kRefresh] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRefresh));
        }
    }

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
}

void WorkspaceMenuScene::updateState(QMenu *parent)
{
    if (d->isEmptyArea) {
        d->sortMenuAction(parent, d->emptyMenuActionRule());
    } else {
        d->sortMenuAction(parent, d->normalMenuActionRule());
    }

    AbstractMenuScene::updateState(parent);
}

bool WorkspaceMenuScene::triggered(QAction *action)
{
    if (d->isEmptyArea)
        return emptyMenuTriggered(action);

    return normalMenuTriggered(action);
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

    if (sceneName == WorkspaceMenuCreator::name()) {
        // refresh
        if (actionId == ActionID::kRefresh) {
            d->view->refresh();
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}

bool WorkspaceMenuScene::normalMenuTriggered(QAction *action)
{
    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    auto actionScene = scene(action);
    if (!actionScene) {
        qWarning() << actionId << " doesn't belong to any scene.";
        return false;
    }

    const QString &sceneName = actionScene->name();
    if (sceneName == kFileOperatorMenuSceneName) {
        // rename
        if (actionId == dfmplugin_menu::ActionID::kRename) {
            if (1 == d->selectFiles.count()) {
                const QModelIndex &index = d->view->selectionModel()->currentIndex();
                if (Q_UNLIKELY(!index.isValid()))
                    return false;
                d->view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
            } else {
                WorkspaceEventCaller::sendShowCustomTopWidget(d->windowId, Global::kFile, true);
            }
            return true;
        }
    } else if (sceneName == kOpenDirMenuSceneName) {
        if (actionId == dfmplugin_menu::ActionID::kOpenInNewTab) {
            WorkspaceHelper::instance()->actionNewTab(d->windowId, d->focusFile);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
