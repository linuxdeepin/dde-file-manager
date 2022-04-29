/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "canvasmenuscene_p.h"
#include "canvasmenu_defines.h"

#include "displayconfig.h"
#include "canvasmanager.h"
#include "grid/canvasgrid.h"
#include "view/canvasview.h"
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"
#include "delegate/canvasitemdelegate.h"
#include "view/operator/fileoperatorproxy.h"
#include "utils/renamedialog.h"

#include <services/common/menu/menu_defines.h>
#include <services/common/menu/menuservice.h>
#include <plugins/common/dfmplugin-menu/dfmplugin_menu_global.h>
#include <plugins/common/dfmplugin-menu/menuscene/action_defines.h>

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusPendingCall>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPMENU_USE_NAMESPACE

static const char *const kNewCreateMenuSceneName = "NewCreateMenu";
static const char *const kClipBoardMenuSceneName = "ClipBoardMenu";
static const char *const kOpenWithMenuSceneName = "OpenWithMenu";
static const char *const kFileOperatorMenuSceneName = "FileOperatorMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";
static const char *const kExtendMenuSceneName = "ExtendMenu";
static const char *const kOemMenuSceneName = "OemMenu";

AbstractMenuScene *CanvasMenuCreator::create()
{
    return new CanvasMenuScene();
}

CanvasMenuScenePrivate::CanvasMenuScenePrivate(CanvasMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
    // 获取菜单服务
    menuServer = MenuService::service();

    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenAsAdmin);

    normalDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewTab);
    normalDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
}

void CanvasMenuScenePrivate::filterDisableAction(QMenu *menu)
{
    // remove disable action
    auto actions = menu->actions();
    QMultiHash<QString, QString> *disableActions = nullptr;
    if (isEmptyArea)
        disableActions = &emptyDisableActions;
    else
        disableActions = &normalDisableActions;

    if (!disableActions->isEmpty()) {
        for (auto action : actions) {
            if (action->isSeparator())
                continue;

            auto actionScene = q->scene(action);
            if (!actionScene) {
                // no scene,remove it.
                menu->removeAction(action);
                continue;
            }

            auto sceneName = actionScene->name();
            auto actionId = action->property(ActionPropertyKey::kActionID).toString();

            if (disableActions->contains(sceneName, actionId)) {
                // disable,remove it.
                menu->removeAction(action);
            }
        }
    }
}

void CanvasMenuScenePrivate::sortMenuAction(QMenu *menu, const QStringList &sortRule)
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
    std::function<void(int)> insertSeparator;
    insertSeparator = [&](int index) {
        if (index >= sortRule.size() || dfmplugin_menu::ActionID::kSeparator == sortRule[index])
            return;

        auto rule = sortRule[index];
        auto iter = std::find_if(actions.begin(), actions.end(), [&rule](const QAction *act) {
            auto p = act->property(ActionPropertyKey::kActionID);
            return p == rule;
        });

        if (iter != actions.end()) {
            QAction *separatorAct = new QAction(menu);
            separatorAct->setSeparator(true);
            actions.insert(iter, separatorAct);
        } else {
            insertSeparator(++index);
        }
    };

    int index = sortRule.indexOf(dfmplugin_menu::ActionID::kSeparator);
    while (-1 != index) {
        insertSeparator(++index);
        index = sortRule.indexOf(dfmplugin_menu::ActionID::kSeparator, index);
    }

    menu->addActions(actions);
}

CanvasMenuScene::CanvasMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new CanvasMenuScenePrivate(this))
{
    d->predicateName[ActionID::kSortBy] = tr("Sort by");
    d->predicateName[ActionID::kIconSize] = tr("Icon size");
    d->predicateName[ActionID::kAutoArrange] = tr("Auto arrange");
    d->predicateName[ActionID::kDisplaySettings] = tr("Display Settings");
    d->predicateName[ActionID::kWallpaperSettings] = tr("Wallpaper and Screensaver");

    // 排序子菜单
    d->predicateName[ActionID::kSrtName] = tr("Name");
    d->predicateName[ActionID::kSrtTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kSrtSize] = tr("Size");
    d->predicateName[ActionID::kSrtType] = tr("Type");
}

QString CanvasMenuScene::name() const
{
    return CanvasMenuCreator::name();
}

bool CanvasMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toString();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->gridPos = params.value(CanvasMenuParams::kDesktopGridPos).toPoint();

    if (d->currentDir.isEmpty())
        return false;

    // todo(wangcl):handle computer,home,trash.and custom menu.

    QList<AbstractMenuScene *> currentScene;
    if (d->isEmptyArea) {
        // new (new doc, new dir)
        if (auto newCreateScene = d->menuServer->createScene(kNewCreateMenuSceneName))
            currentScene.append(newCreateScene);

        // file operation
        if (auto operationScene = d->menuServer->createScene(kClipBoardMenuSceneName))
            currentScene.append(operationScene);

    } else {

        // open with
        if (auto openWithScene = d->menuServer->createScene(kOpenWithMenuSceneName))
            currentScene.append(openWithScene);

        // file operation
        if (auto operationScene = d->menuServer->createScene(kClipBoardMenuSceneName))
            currentScene.append(operationScene);

        // file (open, rename, delete)
        if (auto fileScene = d->menuServer->createScene(kFileOperatorMenuSceneName))
            currentScene.append(fileScene);

        if (auto sendToScene = d->menuServer->createScene(kSendToMenuSceneName))
            currentScene.append(sendToScene);
    }

    // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
    if (auto dirScene = d->menuServer->createScene(kOpenDirMenuSceneName))
        currentScene.append(dirScene);

    // oem menu
    if (auto oemScene = d->menuServer->createScene(kOemMenuSceneName))
        currentScene.append(oemScene);

    // extend menu.must last
    if (auto extendScene = d->menuServer->createScene(kExtendMenuSceneName))
        currentScene.append(extendScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    AbstractMenuScene::initialize(params);

    return true;
}

AbstractMenuScene *CanvasMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<CanvasMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool CanvasMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    d->view = qobject_cast<CanvasView *>(parent->parent());

    if (d->isEmptyArea) {
        this->emptyMenu(parent);
    } else {
        this->normalMenu(parent);
    }

    // 创建子场景菜单
    AbstractMenuScene::create(parent);

    d->filterDisableAction(parent);

    return true;
}

void CanvasMenuScene::updateState(QMenu *parent)
{
    if (d->isEmptyArea)
        d->sortMenuAction(parent, d->emptyMenuActionRules());
    else
        d->sortMenuAction(parent, d->normalMenuActionRules());

    AbstractMenuScene::updateState(parent);
}

bool CanvasMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();

    if (d->predicateAction.values().contains(action)) {
        // sort by
        {
            static const QMap<QString, Global::ItemRoles> sortRole = {
                { ActionID::kSrtName, Global::ItemRoles::kItemFileDisplayNameRole },
                { ActionID::kSrtSize, Global::ItemRoles::kItemFileSizeRole },
                { ActionID::kSrtType, Global::ItemRoles::kItemFileMimeTypeRole },
                { ActionID::kSrtTimeModified, Global::ItemRoles::kItemFileLastModifiedRole }
            };

            if (sortRole.contains(actionId)) {
                Global::ItemRoles role = sortRole.value(actionId);
                Qt::SortOrder order = d->view->model()->sortOrder();
                if (role == d->view->model()->sortRole())
                    order = order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                d->view->model()->setSortRole(role, order);
                d->view->model()->sort();

                // save config
                DispalyIns->setSortMethod(role, order);
                return true;
            }
        }

        // icon size
        if (d->iconSizeAction.contains(action)) {
            int iconLv = d->iconSizeAction.value(action);
            for (auto v : ddplugin_canvas::CanvasIns->views()) {
                v->itemDelegate()->setIconLevel(iconLv);
                v->updateGrid();
            }

            DispalyIns->setIconLevel(iconLv);
            return true;
        }

        // Display Settings
        if (actionId == ActionID::kDisplaySettings) {
            QDBusInterface interface("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter");
            interface.asyncCall("ShowModule", QVariant::fromValue(QString("display")));
            return true;
        }

        // Wallpaper and Screensaver
        if (actionId == ActionID::kWallpaperSettings) {
            CanvasIns->onWallperSetting(d->view);
            return true;
        }

        // Auto arrange
        if (actionId == ActionID::kAutoArrange) {
            bool align = action->isChecked();
            DispalyIns->setAutoAlign(align);
            GridIns->setMode(align ? CanvasGrid::Mode::Align : CanvasGrid::Mode::Custom);
            if (align) {
                GridIns->arrange();
                CanvasIns->update();
            }
            return true;
        }

        qWarning() << "Note:" << actionId << " is belong to screen scene,but not handled.";

    } else {
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
                FileOperatorProxyIns->pasteFiles(d->view, d->gridPos);
                return true;
            }
        }

        // NewCreateMenu scene
        if (sceneName == kNewCreateMenuSceneName) {
            // new folder
            if (actionId == dfmplugin_menu::ActionID::kNewFolder) {
                FileOperatorProxyIns->touchFolder(d->view, d->gridPos);
                return true;
            }

            // new office text
            if (actionId == dfmplugin_menu::ActionID::kNewOfficeText) {
                FileOperatorProxyIns->touchFile(d->view, d->gridPos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord);
                return true;
            }

            // new spreadsheets
            if (actionId == dfmplugin_menu::ActionID::kNewSpreadsheets) {
                FileOperatorProxyIns->touchFile(d->view, d->gridPos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeExcel);
                return true;
            }

            // new presentation
            if (actionId == dfmplugin_menu::ActionID::kNewPresentation) {
                FileOperatorProxyIns->touchFile(d->view, d->gridPos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypePowerpoint);
                return true;
            }

            // new plain text
            if (actionId == dfmplugin_menu::ActionID::kNewPlainText) {
                FileOperatorProxyIns->touchFile(d->view, d->gridPos, DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText);
                return true;
            }
        }

        // FileOperatorMenu scene
        if (sceneName == kFileOperatorMenuSceneName) {
            // rename
            if (actionId == dfmplugin_menu::ActionID::kRename) {
                if (1 == d->selectFiles.count()) {
                    auto index = d->view->model()->index(d->focusFile);
                    if (Q_UNLIKELY(!index.isValid()))
                        return false;
                    d->view->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
                } else {
                    RenameDialog renameDlg(d->selectFiles.count());
                    renameDlg.moveToCenter();

                    // see DDialog::exec,it will return the index of buttons
                    if (1 == renameDlg.exec()) {
                        RenameDialog::ModifyMode mode = renameDlg.modifyMode();
                        if (RenameDialog::kReplace == mode) {
                            auto content = renameDlg.getReplaceContent();
                            FileOperatorProxyIns->renameFiles(d->view, d->selectFiles, content, true);
                        } else if (RenameDialog::kAdd == mode) {
                            auto content = renameDlg.getAddContent();
                            FileOperatorProxyIns->renameFiles(d->view, d->selectFiles, content);
                        } else if (RenameDialog::kCustom == mode) {
                            auto content = renameDlg.getCustomContent();
                            FileOperatorProxyIns->renameFiles(d->view, d->selectFiles, content, false);
                        }
                    }
                }
                return true;
            }
        }
    }

    return AbstractMenuScene::triggered(action);
}

void CanvasMenuScene::emptyMenu(QMenu *parent)
{
    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kSortBy));
    tempAction->setMenu(sortBySubActions(parent));
    d->predicateAction[ActionID::kSortBy] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSortBy));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kIconSize));
    tempAction->setMenu(iconSizeSubActions(parent));
    d->predicateAction[ActionID::kIconSize] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kIconSize));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kAutoArrange));
    tempAction->setCheckable(true);
    tempAction->setChecked(DispalyIns->autoAlign());
    d->predicateAction[ActionID::kAutoArrange] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kAutoArrange));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kDisplaySettings));
    d->predicateAction[ActionID::kDisplaySettings] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySettings));

    //todo update text when screensaver is disbale.
    tempAction = parent->addAction(d->predicateName.value(ActionID::kWallpaperSettings));
    d->predicateAction[ActionID::kWallpaperSettings] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kWallpaperSettings));
}

void CanvasMenuScene::normalMenu(QMenu *parent) {
    Q_UNUSED(parent)
}

QMenu *CanvasMenuScene::iconSizeSubActions(QMenu *menu)
{
    int mininum = d->view->itemDelegate()->minimumIconLevel();
    int maxinum = d->view->itemDelegate()->maximumIconLevel();

    QMenu *subMenu = new QMenu(menu);
    d->iconSizeAction.clear();

    for (int i = mininum; i <= maxinum; ++i) {
        const QString &text = d->view->itemDelegate()->iconSizeLevelDescription(i);
        QAction *tempAction = subMenu->addAction(text);
        tempAction->setCheckable(true);
        tempAction->setChecked(i == d->view->itemDelegate()->iconLevel());
        d->iconSizeAction.insert(tempAction, i);
        d->predicateAction[text] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, text);
    }
    return subMenu;
}

QMenu *CanvasMenuScene::sortBySubActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    // SortBy
    QAction *tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtName));
    d->predicateAction[ActionID::kSrtName] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtName));

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtTimeModified));
    d->predicateAction[ActionID::kSrtTimeModified] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtTimeModified));

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtSize));
    d->predicateAction[ActionID::kSrtSize] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtSize));

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtType));
    d->predicateAction[ActionID::kSrtType] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtType));

    return subMenu;
}
