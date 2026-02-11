// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include "desktoputils/widgetutil.h"

#include "plugins/common/dfmplugin-menu/dfmplugin_menu_global.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <DSysInfo>

#include <QMenu>
#include <QVariant>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusMessage>

using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE
DPMENU_USE_NAMESPACE

static const char *const kActionIconMenuSceneName = "ActionIconManager";
static const char *const kDConfigHiddenMenuSceneName = "DConfigMenuFilter";
static const char *const kCanvasBaseSortMenuSceneName = "CanvasBaseSortMenu";
static const char *const kNewCreateMenuSceneName = "NewCreateMenu";
static const char *const kTemplateMenuSceneName = "TemplateMenu";
static const char *const kClipBoardMenuSceneName = "ClipBoardMenu";
static const char *const kOpenWithMenuSceneName = "OpenWithMenu";
static const char *const kFileOperatorMenuSceneName = "FileOperatorMenu";
static const char *const kSendToMenuSceneName = "SendToMenu";
static const char *const kShareMenuSceneName = "ShareMenu";
static const char *const kOpenDirMenuSceneName = "OpenDirMenu";
static const char *const kExtendMenuSceneName = "ExtendMenu";
static const char *const kOemMenuSceneName = "OemMenu";
static const char *const kBookmarkSceneName = "BookmarkMenu";
static const char *const kPorpertySceneName = "PropertyMenu";

AbstractMenuScene *CanvasMenuCreator::create()
{
    return new CanvasMenuScene();
}

CanvasMenuScenePrivate::CanvasMenuScenePrivate(CanvasMenuScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
    emptyDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenAsAdmin);
    emptyDisableActions.insert(kPorpertySceneName, "property");

    normalDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewTab);
    normalDisableActions.insert(kOpenDirMenuSceneName, dfmplugin_menu::ActionID::kOpenInNewWindow);
    normalDisableActions.insert(kBookmarkSceneName, "add-bookmark");
    normalDisableActions.insert(kBookmarkSceneName, "remove-bookmark");
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

    bool renameEnabled = true;
    if (focusFileInfo && FileUtils::isDesktopFileSuffix(focusFileInfo->fileUrl())
        && !focusFileInfo->canAttributes(CanableInfoType::kCanRename))
        renameEnabled = false;

    if (!disableActions->isEmpty()) {
        for (auto action : actions) {
            if (action->isSeparator())
                continue;

            auto actionScene = q->scene(action);
            if (!actionScene)
                continue;

            auto sceneName = actionScene->name();
            auto actionId = action->property(ActionPropertyKey::kActionID).toString();

            if (disableActions->contains(sceneName, actionId)) {
                // disable,remove it.
                menu->removeAction(action);
            }
            if (actionId == dfmplugin_menu::ActionID::kRename)
                action->setEnabled(renameEnabled);
        }
    }
}

bool CanvasMenuScenePrivate::checkOrganizerPlugin()
{
    return !DPF_NAMESPACE::LifeCycle::blackList().contains("ddplugin-organizer");
}

CanvasMenuScene::CanvasMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new CanvasMenuScenePrivate(this))
{
    d->predicateName[ActionID::kSortBy] = tr("Sort by");
    d->predicateName[ActionID::kDisplaySettings] = tr("Display Settings");
    d->predicateName[ActionID::kRefresh] = tr("Refresh");

    // 1071: removed
    d->predicateName[ActionID::kIconSize] = tr("Icon size");
    d->predicateName[ActionID::kAutoArrange] = tr("Auto arrange");

    if (ddplugin_desktop_util::enableScreensaver()) {
#ifdef COMPILE_ON_V2X
        if (SysInfoUtils::isDeepin23()) {
            d->predicateName[ActionID::kWallpaperSettings] = tr("Wallpaper and Screensaver");
        } else {
            d->predicateName[ActionID::kWallpaperSettings] = tr("Set Wallpaper");
        }
#else
        d->predicateName[ActionID::kWallpaperSettings] = tr("Personalization");
#endif
    } else {
        d->predicateName[ActionID::kWallpaperSettings] = tr("Set Wallpaper");
    }
    // 排序子菜单
    d->predicateName[ActionID::kSrtName] = tr("Name");
    d->predicateName[ActionID::kSrtTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kSrtTimeCreated] = tr("Time created");
    d->predicateName[ActionID::kSrtSize] = tr("Size");
    d->predicateName[ActionID::kSrtType] = tr("Type");

    // 1071: removed
    // subactions of icon size
    d->predicateName[ActionID::kIconSizeTiny] = tr("Tiny");
    d->predicateName[ActionID::kIconSizeSmall] = tr("Small");
    d->predicateName[ActionID::kIconSizeMedium] = tr("Medium");
    d->predicateName[ActionID::kIconSizeLarge] = tr("Large");
    d->predicateName[ActionID::kIconSizeSuperLarge] = tr("Super large");
}

QString CanvasMenuScene::name() const
{
    return CanvasMenuCreator::name();
}

bool CanvasMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty()) {
        d->focusFile = d->selectFiles.first();
        d->focusFileInfo = InfoFactory::create<FileInfo>(d->focusFile);
    }
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->indexFlags = params.value(MenuParamKey::kIndexFlags).value<Qt::ItemFlags>();
    d->gridPos = params.value(CanvasMenuParams::kDesktopGridPos).toPoint();

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();

    d->view = reinterpret_cast<CanvasView *>(params.value(CanvasMenuParams::kDesktopCanvasView).toLongLong());
    if (d->currentDir.isEmpty()) {
        fmCritical() << "Current directory is empty, cannot initialize menu";
        return false;
    }

    QList<AbstractMenuScene *> currentScene;
    // sort
    if (auto sortScene = dfmplugin_menu_util::menuSceneCreateScene(kCanvasBaseSortMenuSceneName)) {
        currentScene.append(sortScene);
        fmDebug() << "Added sort menu scene";
    }

    if (d->isEmptyArea) {
        // new (new doc, new dir)
        if (auto newCreateScene = dfmplugin_menu_util::menuSceneCreateScene(kNewCreateMenuSceneName)) {
            currentScene.append(newCreateScene);
            fmDebug() << "Added new create menu scene";
        }

        // file operation
        if (auto operationScene = dfmplugin_menu_util::menuSceneCreateScene(kClipBoardMenuSceneName)) {
            currentScene.append(operationScene);
            fmDebug() << "Added clipboard menu scene";
        }

    } else {

        // open with
        if (auto openWithScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenWithMenuSceneName)) {
            currentScene.append(openWithScene);
            fmDebug() << "Added open with menu scene";
        }

        // file operation
        if (auto operationScene = dfmplugin_menu_util::menuSceneCreateScene(kClipBoardMenuSceneName)) {
            currentScene.append(operationScene);
            fmDebug() << "Added clipboard menu scene";
        }

        // file (open, rename, delete)
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kFileOperatorMenuSceneName)) {
            currentScene.append(fileScene);
            fmDebug() << "Added file operator menu scene";
        }

        // send to
        if (auto fileScene = dfmplugin_menu_util::menuSceneCreateScene(kSendToMenuSceneName)) {
            currentScene.append(fileScene);
            fmDebug() << "Added send to menu scene";
        }

        if (auto shareScene = dfmplugin_menu_util::menuSceneCreateScene(kShareMenuSceneName)) {
            currentScene.append(shareScene);
            fmDebug() << "Added share menu scene";
        }
    }

    // dir (open in new window,open as admin, open in new tab,open new terminal,select all)
    if (auto dirScene = dfmplugin_menu_util::menuSceneCreateScene(kOpenDirMenuSceneName)) {
        currentScene.append(dirScene);
        fmDebug() << "Added open dir menu scene";
    }

    if (!d->isDDEDesktopFileIncluded) {
        // oem menu
        if (auto oemScene = dfmplugin_menu_util::menuSceneCreateScene(kOemMenuSceneName)) {
            currentScene.append(oemScene);
            fmDebug() << "Added OEM menu scene";
        }

        // extend menu.must last
        if (auto extendScene = dfmplugin_menu_util::menuSceneCreateScene(kExtendMenuSceneName)) {
            currentScene.append(extendScene);
            fmDebug() << "Added extend menu scene";
        }
    }

    if (auto dconfigFilterScene = dfmplugin_menu_util::menuSceneCreateScene(kDConfigHiddenMenuSceneName)) {
        currentScene.append(dconfigFilterScene);
        fmDebug() << "Added DConfig filter scene";
    }

    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene(kActionIconMenuSceneName)) {
        currentScene.append(actionIconManagerScene);
        fmDebug() << "Added action icon manager scene";
    }

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
    if (!parent) {
        fmCritical() << "Null parent menu provided for menu creation";
        return false;
    }

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
    AbstractMenuScene::updateState(parent);
}

bool CanvasMenuScene::triggered(QAction *action)
{
    if (filterActionBySubscene(this, action))
        return true;

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.values().contains(action)) {
        // sort by
        {
            static const QMap<QString, Global::ItemRoles> sortRole = {
                { ActionID::kSrtName, Global::ItemRoles::kItemFileDisplayNameRole },
                { ActionID::kSrtSize, Global::ItemRoles::kItemFileSizeRole },
                { ActionID::kSrtType, Global::ItemRoles::kItemFileMimeTypeRole },
                { ActionID::kSrtTimeModified, Global::ItemRoles::kItemFileLastModifiedRole },
                { ActionID::kSrtTimeCreated, Global::ItemRoles::kItemFileCreatedRole },
            };

            if (sortRole.contains(actionId)) {
                Global::ItemRoles role = sortRole.value(actionId);
                Qt::SortOrder order = d->view->model()->sortOrder();
                if (role == d->view->model()->sortRole())
                    order = order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                fmInfo() << "Sorting items by role:" << static_cast<int>(role) << "order:" << order;
                d->view->model()->setSortRole(role, order);
                d->view->model()->sort();

                // save config
                DispalyIns->setSortMethod(role, order);
                return true;
            }
        }

        // 1071: removed
        // icon size
        if (d->iconSizeAction.contains(action)) {
            CanvasIns->setIconLevel(d->iconSizeAction.value(action));
            return true;
        }

        // Display Settings
        if (actionId == ActionID::kDisplaySettings) {
            fmDebug() << "Opening display settings via ControlCenter";
#ifdef COMPILE_ON_V2X
            QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1",
                                                              "org.deepin.dde.ControlCenter1", "ShowPage");
#else
            QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter",
                                                              "com.deepin.dde.ControlCenter", "ShowModule");

#endif
            msg.setArguments({ QVariant::fromValue(QString("display")) });
            QDBusConnection::sessionBus().asyncCall(msg, 5);
            fmInfo() << "ControlCenter service called - service:" << msg.service() << "arguments:" << msg.arguments();
            return true;
        }

        // refresh
        if (actionId == ActionID::kRefresh) {
            d->view->refresh(false);
            return true;
        }

        // Wallpaper and Screensaver
        if (actionId == ActionID::kWallpaperSettings) {
            CanvasIns->onWallperSetting(d->view);
            return true;
        }

        // 1071: removed
        // Auto arrange
        if (actionId == ActionID::kAutoArrange) {
            bool align = action->isChecked();
            fmInfo() << "Setting auto arrange to:" << align;
            CanvasIns->setAutoArrange(align);
            return true;
        }

        fmWarning() << "Action" << actionId << "belongs to canvas scene but not handled";

    } else {
        auto actionScene = scene(action);
        if (!actionScene) {
            fmWarning() << "Action" << actionId << "does not belong to any scene";
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

            if (actionId == dfmplugin_menu::ActionID::kReverseSelect) {
                d->view->toggleSelect();
                return true;
            }
        }

        // ClipBoardMenu scene
        if (sceneName == kClipBoardMenuSceneName) {
            // paste
            if (actionId == dfmplugin_menu::ActionID::kPaste) {
                auto point = d->gridPos;
                QPointer<ddplugin_canvas::CanvasView> viewptr = d->view;
                QTimer::singleShot(200, [viewptr, point]() {
                    if (!viewptr.isNull())
                        FileOperatorProxyIns->pasteFiles(viewptr, point);
                });
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

        // TemplateMenu scene
        if (sceneName == kTemplateMenuSceneName) {
            FileOperatorProxyIns->touchFile(d->view, d->gridPos, QUrl::fromLocalFile(action->data().toString()));
            return true;
        }

        // FileOperatorMenu scene
        if (sceneName == kFileOperatorMenuSceneName) {
            // rename
            if (actionId == dfmplugin_menu::ActionID::kRename) {
                if (1 == d->selectFiles.count()) {
                    auto index = d->view->model()->index(d->focusFile);
                    if (Q_UNLIKELY(!index.isValid())) {
                        fmWarning() << "Invalid model index for focus file:" << d->focusFile;
                        return false;
                    }
                    QPointer<ddplugin_canvas::CanvasView> view = d->view;
                    QTimer::singleShot(80, [view, index]() {
                        if (!view.isNull())
                            view->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
                    });
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

    ///////////
    // 1071: removed
    if (!d->checkOrganizerPlugin()) {
        tempAction = parent->addAction(d->predicateName.value(ActionID::kIconSize));
        tempAction->setMenu(iconSizeSubActions(parent));
        d->predicateAction[ActionID::kIconSize] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kIconSize));

        tempAction = parent->addAction(d->predicateName.value(ActionID::kAutoArrange));
        tempAction->setCheckable(true);
        tempAction->setChecked(DispalyIns->autoAlign());
        d->predicateAction[ActionID::kAutoArrange] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kAutoArrange));
    }
    ///////////

    tempAction = parent->addAction(d->predicateName.value(ActionID::kDisplaySettings));
    d->predicateAction[ActionID::kDisplaySettings] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySettings));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kRefresh));
    d->predicateAction[ActionID::kRefresh] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kRefresh));

    // todo update text when screensaver is disbale.
    tempAction = parent->addAction(d->predicateName.value(ActionID::kWallpaperSettings));
    d->predicateAction[ActionID::kWallpaperSettings] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kWallpaperSettings));
}

void CanvasMenuScene::normalMenu(QMenu *parent)
{
    Q_UNUSED(parent)
}

QMenu *CanvasMenuScene::iconSizeSubActions(QMenu *menu)
{
    int mininum = d->view->itemDelegate()->minimumIconLevel();
    int maxinum = d->view->itemDelegate()->maximumIconLevel();
    Q_ASSERT(mininum == 0);

    const QStringList keys { ActionID::kIconSizeTiny,
                             ActionID::kIconSizeSmall, ActionID::kIconSizeMedium,
                             ActionID::kIconSizeLarge, ActionID::kIconSizeSuperLarge };
    Q_ASSERT(maxinum == keys.size() - 1);

    QMenu *subMenu = new QMenu(menu);
    d->iconSizeAction.clear();
    int current = d->view->itemDelegate()->iconLevel();
    for (int i = mininum; i <= maxinum; ++i) {
        const QString &key = keys.at(i);
        QAction *tempAction = subMenu->addAction(d->predicateName.value(key));
        tempAction->setCheckable(true);
        tempAction->setChecked(i == current);
        d->iconSizeAction.insert(tempAction, i);
        d->predicateAction[key] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, key);
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

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtTimeCreated));
    d->predicateAction[ActionID::kSrtTimeCreated] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtTimeCreated));

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtSize));
    d->predicateAction[ActionID::kSrtSize] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtSize));

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kSrtType));
    d->predicateAction[ActionID::kSrtType] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtType));

    return subMenu;
}
