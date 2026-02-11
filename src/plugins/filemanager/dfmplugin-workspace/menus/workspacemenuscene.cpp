// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacemenuscene.h"
#include "workspacemenuscene_p.h"
#include "events/workspaceeventcaller.h"

#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "models/fileviewmodel.h"
#include "utils/fileoperatorhelper.h"
#include "views/workspacewidget.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>

#include <DMenu>

using namespace dfmplugin_workspace;
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

static const char *const kActionIconMenuSceneName = "ActionIconManager";
static const char *const kDConfigHiddenMenuSceneName = "DConfigMenuFilter";
static const char *const kBaseSortMenuSceneName = "BaseSortMenu";
static const char *const kNewCreateMenuSceneName = "NewCreateMenu";
static const char *const kTemplateMenuSceneName = "TemplateMenu";
static const char *const kClipBoardMenuSceneName = "ClipBoardMenu";
static const char *const kOpenWithMenuSceneName = "OpenWithMenu";
static const char *const kFileOperatorMenuSceneName = "FileOperatorMenu";
static const char *const kShareMenuSceneName = "ShareMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";
static const char *const kExtendMenuSceneName = "ExtendMenu";
static const char *const kOemMenuSceneName = "OemMenu";

AbstractMenuScene *WorkspaceMenuCreator::create()
{
    fmDebug() << "Creating WorkspaceMenuScene instance";
    return new WorkspaceMenuScene();
}

WorkspaceMenuScenePrivate::WorkspaceMenuScenePrivate(WorkspaceMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

WorkspaceMenuScene::WorkspaceMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new WorkspaceMenuScenePrivate(this))
{
    fmDebug() << "WorkspaceMenuScene initialized";
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

    fmDebug() << "Initializing WorkspaceMenuScene - currentDir:" << d->currentDir.toString()
              << "selectFiles count:" << d->selectFiles.size() << "isEmptyArea:" << d->isEmptyArea
              << "windowId:" << d->windowId;

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();

    if (d->currentDir.isEmpty()) {
        fmWarning() << "WorkspaceMenuScene initialization failed: currentDir is empty";
        return false;
    }

    QList<AbstractMenuScene *> currentScene;
    // sort
    if (auto sortScene = dfmplugin_menu_util::menuSceneCreateScene(kBaseSortMenuSceneName)) {
        currentScene.append(sortScene);
        fmDebug() << "Added BaseSortMenu scene";
    }

    // file operation
    if (auto operationScene = dfmplugin_menu_util::menuSceneCreateScene(kClipBoardMenuSceneName)) {
        currentScene.append(operationScene);
        fmDebug() << "Added ClipBoardMenu scene";
    }

    // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
    if (auto dirScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenDirMenuSceneName)) {
        currentScene.append(dirScene);
        fmDebug() << "Added OpenDirMenu scene";
    }

    if (d->isEmptyArea) {
        fmDebug() << "Creating empty area menu scenes";
        // new (new doc, new dir)
        if (auto newCreateScene = dfmplugin_menu_util::menuSceneCreateScene(kNewCreateMenuSceneName)) {
            currentScene.append(newCreateScene);
            fmDebug() << "Added NewCreateMenu scene for empty area";
        }
    } else {
        fmDebug() << "Creating normal menu scenes";
        // open with
        if (auto openWithScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenWithMenuSceneName)) {
            currentScene.append(openWithScene);
            fmDebug() << "Added OpenWithMenu scene";
        }

        // file (open, rename, delete)
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kFileOperatorMenuSceneName)) {
            currentScene.append(fileScene);
            fmDebug() << "Added FileOperatorMenu scene";
        }

        // send to
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kSendToMenuSceneName)) {
            currentScene.append(fileScene);
            fmDebug() << "Added SendToMenu scene";
        }

        // share menu
        if (auto shareScene = dfmplugin_menu_util::menuSceneCreateScene(kShareMenuSceneName)) {
            currentScene.append(shareScene);
            fmDebug() << "Added ShareMenu scene";
        }
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);

    // 为了能在扩展菜单中获得所有actions, 扩展菜单场景必须在其他场景之后
    if (!d->isDDEDesktopFileIncluded) {
        // oem menu
        if (auto oemScene = dfmplugin_menu_util::menuSceneCreateScene(kOemMenuSceneName)) {
            currentScene.append(oemScene);
            fmDebug() << "Added OemMenu scene";
        }

        // extend menu.must last
        if (auto extendScene = dfmplugin_menu_util::menuSceneCreateScene(kExtendMenuSceneName)) {
            currentScene.append(extendScene);
            fmDebug() << "Added ExtendMenu scene";
        }
    }

    if (auto dconfigFilterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigHiddenMenuSceneName)) {
        currentScene.append(dconfigFilterScene);
        fmDebug() << "Added DConfigHiddenMenu scene";
    }

    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene(kActionIconMenuSceneName)) {
        currentScene.append(actionIconManagerScene);
        fmDebug() << "Added ActionIconManager scene";
    }

    setSubscene(currentScene);

    // 初始化所有子场景
    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *WorkspaceMenuScene::scene(QAction *action) const
{
    if (!action) {
        fmDebug() << "Cannot find scene: action is null";
        return nullptr;
    }

    if (d->predicateAction.values().contains(action)) {
        fmDebug() << "Action belongs to WorkspaceMenuScene";
        return const_cast<WorkspaceMenuScene *>(this);
    }

    return AbstractMenuScene::scene(action);
}

bool WorkspaceMenuScene::create(DMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot create WorkspaceMenuScene: parent menu is null";
        return false;
    }

    fmDebug() << "Creating workspace menu";
    d->view = qobject_cast<FileView *>(parent->parent());
    Q_ASSERT(d->view);

    if (d->isEmptyArea) {
        fmDebug() << "Adding refresh action for empty area menu";
        auto tempAction = parent->addAction(d->predicateName.value(ActionID::kRefresh));
        d->predicateAction[ActionID::kRefresh] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRefresh));
    }

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
}

void WorkspaceMenuScene::updateState(DMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot update state: parent menu is null";
        return;
    }

    fmDebug() << "Updating workspace menu state";
    auto currentWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(d->windowId);
    if (!currentWidget) {
        fmWarning() << "Cannot find workspace widget for windowId:" << d->windowId;
        AbstractMenuScene::updateState(parent);
        return;
    }

    bool renameEnabled = true;
    if (d->focusFileInfo && FileUtils::isDesktopFileSuffix(d->focusFileInfo->fileUrl())
        && !d->focusFileInfo->canAttributes(CanableInfoType::kCanRename)) {
        renameEnabled = false;
        fmDebug() << "Rename disabled for desktop file:" << d->focusFileInfo->fileUrl().toString();
    }

    bool addTabEnabled = WorkspaceEventCaller::sendCheckTabAddable(d->windowId);
    fmDebug() << "Tab addable:" << addTabEnabled << "rename enabled:" << renameEnabled;

    auto actions = parent->actions();
    for (auto *act : actions) {
        const auto &actId = act->property(ActionPropertyKey::kActionID);
        if (dfmplugin_menu::ActionID::kOpenInNewTab == actId) {
            act->setEnabled(addTabEnabled);
            fmDebug() << "Updated OpenInNewTab action enabled:" << addTabEnabled;
        } else if (dfmplugin_menu::ActionID::kRename == actId) {
            act->setEnabled(renameEnabled);
            fmDebug() << "Updated Rename action enabled:" << renameEnabled;
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool WorkspaceMenuScene::triggered(QAction *action)
{
    if (!action) {
        fmWarning() << "Cannot trigger action: action is null";
        return false;
    }

    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    fmDebug() << "Action triggered in WorkspaceMenuScene:" << actionId << "isEmptyArea:" << d->isEmptyArea;

    if (filterActionBySubscene(this, action)) {
        fmDebug() << "Action filtered by subscene:" << actionId;
        return true;
    }

    if (d->isEmptyArea) {
        fmDebug() << "Processing empty area menu action:" << actionId;
        return emptyMenuTriggered(action);
    }

    return normalMenuTriggered(action);
}

bool WorkspaceMenuScene::emptyMenuTriggered(QAction *action)
{
    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    fmDebug() << "Processing empty area menu action:" << actionId;

    auto actionScene = scene(action);
    if (!actionScene) {
        fmWarning() << actionId << " doesn't belong to any scene.";
        return false;
    }

    const QString &sceneName = actionScene->name();
    fmDebug() << "Action belongs to scene:" << sceneName;

    // ClipBoardMenu scene
    if (sceneName == kClipBoardMenuSceneName) {
        // paste
        if (actionId == dfmplugin_menu::ActionID::kPaste) {
            fmDebug() << "Executing paste operation in empty area";
            QPointer<dfmplugin_workspace::FileView> view = d->view;
            QTimer::singleShot(200, [view]() {
                if (!view.isNull()) {
                    fmDebug() << "Executing delayed paste operation";
                    FileOperatorHelperIns->pasteFiles(view);
                }
            });
            return true;
        }
    }

    // NewCreateMenu scene
    if (sceneName == kNewCreateMenuSceneName) {
        // new folder
        if (actionId == dfmplugin_menu::ActionID::kNewFolder) {
            fmDebug() << "Creating new folder";
            FileOperatorHelperIns->touchFolder(d->view);
            return true;
        }

        // new office text
        if (actionId == dfmplugin_menu::ActionID::kNewOfficeText) {
            fmDebug() << "Creating new office text document";
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord);
            return true;
        }

        // new spreadsheets
        if (actionId == dfmplugin_menu::ActionID::kNewSpreadsheets) {
            fmDebug() << "Creating new spreadsheet";
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeExcel);
            return true;
        }

        // new presentation
        if (actionId == dfmplugin_menu::ActionID::kNewPresentation) {
            fmDebug() << "Creating new presentation";
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypePowerpoint);
            return true;
        }

        // new plain text
        if (actionId == dfmplugin_menu::ActionID::kNewPlainText) {
            fmDebug() << "Creating new plain text file";
            FileOperatorHelperIns->touchFiles(d->view, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText);
            return true;
        }
    }

    // TemplateMenu scene
    if (sceneName == kTemplateMenuSceneName) {
        fmDebug() << "Creating file from template:" << action->data().toString();
        FileOperatorHelperIns->touchFiles(d->view, QUrl::fromLocalFile(action->data().toString()));
        return true;
    }

    if (sceneName == WorkspaceMenuCreator::name()) {
        // refresh
        if (actionId == ActionID::kRefresh) {
            fmDebug() << "Refreshing workspace view";
            d->view->refresh();
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}

bool WorkspaceMenuScene::normalMenuTriggered(QAction *action)
{
    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    fmDebug() << "Processing normal menu action:" << actionId;

    auto actionScene = scene(action);
    if (!actionScene) {
        fmWarning() << actionId << " doesn't belong to any scene.";
        return false;
    }

    const QString &sceneName = actionScene->name();
    fmDebug() << "Action belongs to scene:" << sceneName;

    if (sceneName == kFileOperatorMenuSceneName) {
        // rename
        if (actionId == dfmplugin_menu::ActionID::kRename) {
            if (1 == d->selectFiles.count()) {
                fmDebug() << "Starting rename operation for single file";

                // 使用 d->focusFile 而不是 selectionModel()->currentIndex()
                // 原因：selectionModel()->currentIndex() 在菜单显示期间可能会改变
                // d->focusFile 是稳定的，表示右键选中的文件
                const QModelIndex &index = d->view->model()->getIndexByUrl(d->focusFile);

                if (Q_UNLIKELY(!index.isValid())) {
                    fmWarning() << "Cannot rename: invalid selection index for focusFile:" << d->focusFile.toString();
                    return false;
                }

                QPointer<dfmplugin_workspace::FileView> view = d->view;
                QTimer::singleShot(80, [view, index]() {
                    if (!view.isNull()) {
                        fmDebug() << "Executing delayed rename operation" << index;
                        view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
                    }
                });
                d->view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
            } else {
                fmInfo() << "Starting batch rename operation for" << d->selectFiles.count() << "files";
                WorkspaceEventCaller::sendShowCustomTopWidget(d->windowId, Global::Scheme::kFile, true);
            }
            return true;
        }
    } else if (sceneName == kOpenDirMenuSceneName) {
        if (actionId == dfmplugin_menu::ActionID::kOpenInNewTab) {
            fmDebug() << "Opening in new tab:" << d->focusFile.toString();
            WorkspaceEventCaller::sendOpenNewTab(d->windowId, d->focusFile);
            return true;
        }
        if (actionId == dfmplugin_menu::ActionID::kReverseSelect) {
            fmDebug() << "Executing reverse select operation";
            WorkspaceHelper::instance()->reverseSelect(d->windowId);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
