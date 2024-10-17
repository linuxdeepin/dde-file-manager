// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

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

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();

    if (d->currentDir.isEmpty())
        return false;

    QList<AbstractMenuScene *> currentScene;
    // sort
    if (auto sortScene = dfmplugin_menu_util::menuSceneCreateScene(kBaseSortMenuSceneName))
        currentScene.append(sortScene);

    // file operation
    if (auto operationScene = dfmplugin_menu_util::menuSceneCreateScene(kClipBoardMenuSceneName))
        currentScene.append(operationScene);

    // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
    if (auto dirScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenDirMenuSceneName))
        currentScene.append(dirScene);

    if (d->isEmptyArea) {
        // new (new doc, new dir)
        if (auto newCreateScene = dfmplugin_menu_util::menuSceneCreateScene(kNewCreateMenuSceneName))
            currentScene.append(newCreateScene);
    } else {
        // open with
        if (auto openWithScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenWithMenuSceneName))
            currentScene.append(openWithScene);

        // file (open, rename, delete)
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kFileOperatorMenuSceneName))
            currentScene.append(fileScene);

        // send to
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kSendToMenuSceneName))
            currentScene.append(fileScene);

        // share menu
        if (auto shareScene = dfmplugin_menu_util::menuSceneCreateScene(kShareMenuSceneName))
            currentScene.append(shareScene);
    }

    if (!d->isDDEDesktopFileIncluded) {
        // oem menu
        if (auto oemScene = dfmplugin_menu_util::menuSceneCreateScene(kOemMenuSceneName))
            currentScene.append(oemScene);

        // extend menu.must last
        if (auto extendScene = dfmplugin_menu_util::menuSceneCreateScene(kExtendMenuSceneName))
            currentScene.append(extendScene);
    }

    if (auto dconfigFilterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigHiddenMenuSceneName))
        currentScene.append(dconfigFilterScene);

    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene(kActionIconMenuSceneName))
        currentScene.append(actionIconManagerScene);

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

bool WorkspaceMenuScene::create(DMenu *parent)
{
    if (!parent)
        return false;

    d->view = qobject_cast<FileView *>(parent->parent());
    Q_ASSERT(d->view);

    if (d->isEmptyArea) {
        auto tempAction = parent->addAction(d->predicateName.value(ActionID::kRefresh));
        d->predicateAction[ActionID::kRefresh] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRefresh));
    }

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
}

void WorkspaceMenuScene::updateState(DMenu *parent)
{
    auto currentWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(d->windowId);
    bool renameEnabled = true;
    if (d->focusFileInfo && FileUtils::isDesktopFileInfo(d->focusFileInfo)
            && !d->focusFileInfo->canAttributes(CanableInfoType::kCanRename))
        renameEnabled = false;

    if (currentWidget && WorkspaceEventCaller::sendCheckTabAddable(d->windowId)) {
        auto actions = parent->actions();
        for (auto act : actions) {
            const auto &actId = act->property(ActionPropertyKey::kActionID);
            if (dfmplugin_menu::ActionID::kOpenInNewTab == actId) {
                act->setEnabled(false);
            } else if (!renameEnabled && dfmplugin_menu::ActionID::kRename == actId){
                act->setEnabled(renameEnabled);
            }
        }
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
        fmWarning() << actionId << " doesn't belong to any scene.";
        return false;
    }

    const QString &sceneName = actionScene->name();
    // ClipBoardMenu scene
    if (sceneName == kClipBoardMenuSceneName) {
        // paste
        if (actionId == dfmplugin_menu::ActionID::kPaste) {
            QPointer<dfmplugin_workspace::FileView> view = d->view;
            QTimer::singleShot(200, [view](){
                if (!view.isNull())
                    FileOperatorHelperIns->pasteFiles(view);
            });
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

    // TemplateMenu scene
    if (sceneName == kTemplateMenuSceneName) {
        FileOperatorHelperIns->touchFiles(d->view, QUrl::fromLocalFile(action->data().toString()));
        return true;
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
        fmWarning() << actionId << " doesn't belong to any scene.";
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
                QPointer<dfmplugin_workspace::FileView> view = d->view;
                QTimer::singleShot(80, [view, index](){
                    if (!view.isNull())
                        view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
                });
                d->view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
            } else {
                WorkspaceEventCaller::sendShowCustomTopWidget(d->windowId, Global::Scheme::kFile, true);
            }
            return true;
        }
    } else if (sceneName == kOpenDirMenuSceneName) {
        if (actionId == dfmplugin_menu::ActionID::kOpenInNewTab) {
            WorkspaceEventCaller::sendOpenNewTab(d->windowId, d->focusFile);
            return true;
        }
        if (actionId == dfmplugin_menu::ActionID::kReverseSelect) {
            WorkspaceHelper::instance()->reverseSelect(d->windowId);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
