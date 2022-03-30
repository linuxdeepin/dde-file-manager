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
#include "canvasmenu.h"
#include "view/canvasview.h"
#include "view/operator/fileoperaterproxy.h"
#include "canvasmanager.h"
#include "model/canvasmodel.h"
#include "model/canvasselectionmodel.h"
#include "utils/renamedialog.h"
#include "displayconfig.h"
#include "grid/canvasgrid.h"
#include "utils/fileutil.h"
#include "delegate/canvasitemdelegate.h"
#include "model/canvasmodel.h"

#include <services/common/bluetooth/bluetoothservice.h>

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_actiontype_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <file/local/localfileinfo.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/actiontypemanager.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/devicemanager.h>
#include <dfm-base/file/fileAction/desktopfileactions.h>

#include <QGSettings>

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QMenu>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

namespace MenuScene {
const char *const kDesktopMenu = "desktop-menu";
}   // namespace MenuScene

CanvasMenu::CanvasMenu()
{
    // 获取菜单服务
    extensionMenuServer = MenuService::service();

    // regist desktop aciton type
    this->registDesktopCustomActions();

    // associate user column Roles associate ActionType for desktop
    this->columnRolesAssociateActionType();
}

CanvasMenu::~CanvasMenu()
{
    if (sortByActionData) {
        delete sortByActionData;
        sortByActionData = nullptr;
    }
}

QMenu *CanvasMenu::build(QWidget *parent,
                         AbstractMenu::MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &focusUrl,
                         const QList<QUrl> &selected,
                         QVariant customData)
{
    cusData = std::move(customData);
    view = qobject_cast<CanvasView *>(parent);
    QMenu *menu = new QMenu(parent);
    if (AbstractMenu::MenuMode::kEmpty == mode) {
        emptyAreaMenu(menu, rootUrl);
    }

    if (AbstractMenu::MenuMode::kNormal == mode) {
        normalMenu(menu, rootUrl, focusUrl, selected);
    }
    return menu;
}

void CanvasMenu::emptyAreaMenu(QMenu *menu, const QUrl &rootUrl)
{
    if (!view)
        return;

    //! todo(lq) unused.
    //    QString errString;
    //    auto tempInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(rootUrl, true, &errString);
    //    if (!tempInfo) {
    //        qInfo() << "create LocalFileInfo error: " << errString;
    //        return;
    //    }

    QVector<ActionDataContainer> tempActDataLst;

    // regest IconSize sub actions
    if (!actionTypesInitialized) {
        registDesktopCustomSubActions();
        actionTypesInitialized = true;
    }

    // Prepare the type of menu item to display

    tempActDataLst << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActNewFolder)
                   << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActNewDocument)
                   << *sortByActionData
                   << customAction.value(DesktopCustomAction::kIconSize)
                   << customAction.value(DesktopCustomAction::kAutoArrange)
                   << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActPaste)
                   << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSelectAll)
                   << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActOpenInTerminal);
    if (isRefreshOn())
        tempActDataLst << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActRefreshView);
    tempActDataLst << ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSeparator)
                   << customAction.value(DesktopCustomAction::kDisplaySettings)
                   << customAction.value(DesktopCustomAction::kWallpaperSettings);

    // add action to menu
    this->creatMenuByDataLst(menu, tempActDataLst);

    // Action special handling
    this->setActionSpecialHandling(menu);
}

ActionDataContainer CanvasMenu::getSendToMenu(bool hasFolder)
{
    ActionDataContainer actSendTo = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSendToRemovableDisk);
    QVector<ActionDataContainer> subActs;

    if (BluetoothService::service() && BluetoothService::service()->bluetoothEnable()) {
        auto act = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSendToBluetooth);
        if (hasFolder)
            ;   // TODO(xust) disable the act
        subActs << act;
    }

    // TODO(xust) optimize
    QStringList blks = DeviceManagerInstance.invokeBlockDevicesIdList({});
    sendToRemovabalDiskActs.clear();
    for (const QString &id : blks) {

        auto info = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
        QString mpt = info.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
        bool optical = info.value(GlobalServerDefines::DeviceProperty::kOptical).toBool();
        QString label = info.value(GlobalServerDefines::DeviceProperty::kIdLabel).toString();
        bool removable = info.value(GlobalServerDefines::DeviceProperty::kRemovable).toBool();
        if (!mpt.isEmpty() && removable) {
            QUrl targetUrl;
            if (optical) {
                // TODO(xust): converted burn url
            } else {
                targetUrl = UrlRoute::pathToReal(mpt);
            }
            auto container = ActionTypeManager::instance().registerActionType("SendTo", label);
            subActs.append(container.second);
            sendToRemovabalDiskActs.insert(container.first, targetUrl);
        }
    }

    actSendTo.setChildrenActionsData(subActs);
    return actSendTo;
}

void CanvasMenu::normalMenu(QMenu *menu,
                            const QUrl &rootUrl,
                            const QUrl &focusUrl,
                            const QList<QUrl> &selected)
{
    Q_UNUSED(rootUrl)

    if (!view)
        return;
// using coverted file info
#if 0
    QString errString;
    auto tempInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(focusUrl, true, &errString);
    if (!tempInfo) {
        qInfo() << "create LocalFileInfo error: " << errString;
        return;
    }
#else
    auto tempInfo = FileCreator->createFileInfo(focusUrl);
    if (Q_UNLIKELY(!tempInfo))
        return;
#endif
    QVector<ActionDataContainer> tempActDataLst;
    // 获取对应Scheme对应的菜单列表
    auto baseDec = QSharedPointer<AbstractFileInfo>(new AbstractFileActions(tempInfo));
    // 加上桌面场景过滤后的文件菜单列表
    AbstractFileActions *tempfileActions = new DesktopFileActions(baseDec);
    QVector<ActionType> tempActTypeList;
    // 过滤场景内enable 和 display 的action
    QSet<ActionType> unUsedActions { ActionType::kActSendToDesktop };
    if (selected.size() == 1) {
        tempActTypeList = tempfileActions->menuActionList(AbtMenuType::kSingleFile);
        unUsedActions << ActionType::kActOpenInNewWindow
                      << ActionType::kActOpenInNewTab;
        // TODO(Lee) ActionType::kActAddToBookMark // 功能缺失
    } else {

        bool isSystemPathIncluded = false;
        for (auto &temp : selected) {
            if (SystemPathUtil::instance()->isSystemPath(temp.url())) {
                isSystemPathIncluded = true;
                break;
            }
        }
        if (isSystemPathIncluded) {
            tempActTypeList = tempfileActions->menuActionList(AbtMenuType::kMultiFilesSystemPathIncluded);
        } else {
            tempActTypeList = tempfileActions->menuActionList(AbtMenuType::kMultiFiles);
        }
    }

    if (tempActTypeList.isEmpty())
        return;

    getActionDataByTypes(tempActDataLst, tempActTypeList, unUsedActions);

    // TODO(xust) adjust the order
    const auto &&sendTo = getSendToMenu(false);
    if (sendTo.childrenActionsData().count() > 0)
        tempActDataLst << sendTo;

    // add action to menu
    this->creatMenuByDataLst(menu, tempActDataLst);
    // add menu filter
}

void CanvasMenu::actionBusiness(QAction *act)
{
    auto actType = act->data().toInt();
    if (customActionType.contains(actType))
        actType = customActionType.value(actType);

    switch (actType) {
    case DesktopCustomAction::kIconSize0:
    case DesktopCustomAction::kIconSize1:
    case DesktopCustomAction::kIconSize2:
    case DesktopCustomAction::kIconSize3:
    case DesktopCustomAction::kIconSize4: {
        int lv = actType - kIconSize;
        for (auto v : CanvasIns->views()) {
            v->itemDelegate()->setIconLevel(lv);
            v->updateGrid();
        }

        DispalyIns->setIconLevel(lv);
        return;
    }
    case ActionType::kActRefreshView: {
        view->refresh();
        return;
    }
    case ActionType::kActSelectAll: {
        view->selectAll();
        return;
    }
    case DesktopCustomAction::kDisplaySettings: {
        QDBusInterface interface("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter");
        interface.asyncCall("ShowModule", QVariant::fromValue(QString("display")));
        return;
    }
    case DesktopCustomAction::kWallpaperSettings: {
        CanvasIns->onWallperSetting(view);
        return;
    }
    case DesktopCustomAction::kAutoArrange: {
        bool align = act->isChecked();
        DispalyIns->setAutoAlign(align);
        GridIns->setMode(align ? CanvasGrid::Mode::Align : CanvasGrid::Mode::Custom);
        if (align) {
            GridIns->arrange();
            CanvasIns->update();
        }
        return;
    }
    case ActionType::kActName:
    case ActionType::kActSize:
    case ActionType::kActType:
    case ActionType::kActLastModifiedDate: {
        auto sortByRole = [=](const DFMBASE_NAMESPACE::AbstractFileInfo::SortKey role) -> bool {
            Qt::SortOrder order = view->model()->sortOrder();
            if (role == view->model()->sortRole())
                order = order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            view->model()->setSortRole(role, order);
            view->model()->sort();

            // save config
            DispalyIns->setSortMethod(role, order);
            return true;
        };

        // sort
        auto sortRole = userColumnRoles.key(DFMBASE_NAMESPACE::ActionType(actType));
        sortByRole(sortRole);
        return;
    }
    case kActNewFolder: {
        FileOperaterProxyIns->touchFolder(view, cusData.toPoint());
        return;
    }
    case kActNewText: {
        FileOperaterProxyIns->touchFile(view, cusData.toPoint(), DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText);
        return;
    }
    case kActNewWord: {
        FileOperaterProxyIns->touchFile(view, cusData.toPoint(), DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord);
        return;
    }
    case kActNewExcel: {
        FileOperaterProxyIns->touchFile(view, cusData.toPoint(), DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeExcel);
        return;
    }
    case kActNewPowerpoint: {
        FileOperaterProxyIns->touchFile(view, cusData.toPoint(), DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypePowerpoint);
        return;
    }
        // TODO: 常规菜单中大部分直接使用基础默认的响应，不做特殊处理，
        //       这里是暂时接入桌面业务,后续待dde-file-manager接入进来后调整清理.
    case kActOpen: {
        FileOperaterProxyIns->openFiles(view);
        return;
    }
        //    case kActOpenWith:
        //    case kActOpenAsAdmin:
    case kActDelete: {
        FileOperaterProxyIns->moveToTrash(view);
        return;
    }
    case ActionType::kActOpenInTerminal: {
        QList<QUrl> urls;
        urls.append(view->model()->rootUrl());
        dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenInTerminal, view->winId(), urls);
        return;
    }
    case kActPaste: {
        FileOperaterProxyIns->pasteFiles(view, cusData.toPoint());
        return;
    }
    case kActCopy: {
        FileOperaterProxyIns->copyFiles(view);
        return;
    }
    case kActRename: {
        auto selected = view->selectionModel()->selectedUrls();
        if (selected.isEmpty())
            return;
        if (1 == selected.size()) {
            auto index = view->model()->index(selected.first());
            if (Q_UNLIKELY(!index.isValid()))
                return;
            view->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
        } else {
            RenameDialog renameDlg(selected.count());
            renameDlg.moveToCenter();

            // see DDialog::exec,it will return the index of buttons
            if (1 == renameDlg.exec()) {
                RenameDialog::ModifyMode mode = renameDlg.modifyMode();
                if (RenameDialog::kReplace == mode) {
                    auto content = renameDlg.getReplaceContent();
                    FileOperaterProxyIns->renameFiles(view, selected, content, true);
                } else if (RenameDialog::kAdd == mode) {
                    auto content = renameDlg.getAddContent();
                    FileOperaterProxyIns->renameFiles(view, selected, content);
                } else if (RenameDialog::kCustom == mode) {
                    auto content = renameDlg.getCustomContent();
                    FileOperaterProxyIns->renameFiles(view, selected, content, false);
                }
            }
        }
        return;
    }
    case kActClearTrash: {
        // TODO(lee):
        return;
    }
    case kActCreateSymlink: {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCreateSymlink,
                                              view->winId(),
                                              view->model()->url(view->currentIndex()));
        return;
    }
    case kActProperty: {
        FileOperaterProxyIns->showFilesProperty(view);
        return;
    }
    case kActSendToBluetooth: {
        FileOperaterProxyIns->sendFilesToBluetooth(view);
        return;
    }
    default:
        break;
    }

    if (sendToRemovabalDiskActs.contains(actType)) {
        const QUrl &target = sendToRemovabalDiskActs.value(actType);
        // TODO(xust): publish copy event.
        qDebug() << "send files to: " << target;
    }

    AbstractMenu::actionBusiness(act);
}

void CanvasMenu::registDesktopCustomActions()
{
    // regist desktop aciton type
    QPair<int, ActionDataContainer> customAct = ActionTypeManager::instance().registerActionType("DisplaySettings", tr("Display Settings"));
    customAction.insert(DesktopCustomAction::kDisplaySettings, customAct.second);
    customActionType.insert(customAct.first, DesktopCustomAction::kDisplaySettings);

    customAct = ActionTypeManager::instance().registerActionType("WallpaperSettings", tr("Wallpaper and Screensaver"));
    customAction.insert(DesktopCustomAction::kWallpaperSettings, customAct.second);
    customActionType.insert(customAct.first, DesktopCustomAction::kWallpaperSettings);

    customAct = ActionTypeManager::instance().registerActionType("IconSize", tr("Icon size"));
    customAction.insert(DesktopCustomAction::kIconSize, customAct.second);
    customActionType.insert(customAct.first, DesktopCustomAction::kIconSize);

    customAct = ActionTypeManager::instance().registerActionType("AutoArrange", tr("Auto arrange"));
    customAction.insert(DesktopCustomAction::kAutoArrange, customAct.second);
    customActionType.insert(customAct.first, DesktopCustomAction::kAutoArrange);
}

/*!
 * \brief CanvasMenu::registDesktopCustomSubActions
 *
 */
void CanvasMenu::registDesktopCustomSubActions()
{
    if (!view)
        return;

    // 特殊处理 kActIconSize
    int mininum = view->itemDelegate()->minimumIconLevel();
    int maxinum = view->itemDelegate()->maximumIconLevel();
    for (int i = mininum; i <= maxinum; ++i) {
        auto actionData = ActionTypeManager::instance().registerActionType(QString("IconSize0%1").arg(i),
                                                                           view->itemDelegate()->iconSizeLevelDescription(i));

        customAction[DesktopCustomAction::kIconSize].addChildrenActionsData(actionData.second);
        customActionType.insert(actionData.first, DesktopCustomAction(DesktopCustomAction::kIconSize + i));
    }

    // 特殊处理 kActSortBy
    auto tempActionData = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSortBy);
    sortByActionData = new ActionDataContainer(tempActionData);
    QVector<ActionDataContainer> tempActionDataLst;
    for (auto roleType : userColumnRoles.values())
        tempActionDataLst << ActionTypeManager::instance().actionDataContainerByType(roleType);
    sortByActionData->setChildrenActionsData(tempActionDataLst);
}

void CanvasMenu::columnRolesAssociateActionType()
{
    // associate user column Roles associate ActionType for desktop
    userColumnRoles.insert(DFMBASE_NAMESPACE::AbstractFileInfo::SortKey::kSortByFileName, DFMBASE_NAMESPACE::ActionType::kActName);
    userColumnRoles.insert(DFMBASE_NAMESPACE::AbstractFileInfo::SortKey::kSortByModified, DFMBASE_NAMESPACE::ActionType::kActLastModifiedDate);
    userColumnRoles.insert(DFMBASE_NAMESPACE::AbstractFileInfo::SortKey::kSortByFileSize, DFMBASE_NAMESPACE::ActionType::kActSize);
    userColumnRoles.insert(DFMBASE_NAMESPACE::AbstractFileInfo::SortKey::kSortByFileMimeType, DFMBASE_NAMESPACE::ActionType::kActType);
}

void CanvasMenu::creatMenuByDataLst(QMenu *menu, const QVector<ActionDataContainer> &lst)
{
    // 根据actionData获取到action信息;
    if (lst.isEmpty())
        return;

    // add action to menu
    for (auto &tempActData : lst) {
        // Separator
        if (tempActData.actionType() == ActionType::kActSeparator) {
            menu->addSeparator();
            continue;
        }

        if (tempActData.name().isEmpty() || (-1 == tempActData.actionType()))
            continue;

        QAction *act = new QAction(menu);
        act->setData(tempActData.actionType());
        act->setText(tempActData.name());
        if (!tempActData.icon().isNull())
            act->setIcon(tempActData.icon());

        // add sub action
        auto subLst = tempActData.childrenActionsData();
        if (subLst.size() > 0) {
            QMenu *subMenu = new QMenu(menu);
            creatMenuByDataLst(subMenu, subLst);
            act->setMenu(subMenu);
        }

        menu->addAction(act);
    }
}

void CanvasMenu::getActionDataByTypes(QVector<DFMBASE_NAMESPACE::ActionDataContainer> &lst,
                                      const QVector<ActionType> &types,
                                      const QSet<ActionType> &unUsedTypes)
{
    for (DFMBASE_NAMESPACE::ActionType type : types) {
        if (unUsedTypes.contains(type))
            continue;
        lst << ActionTypeManager::instance().actionDataContainerByType(type);
    }
}

void CanvasMenu::setActionSpecialHandling(QMenu *menu)
{
    QList<QAction *> actions = menu->actions();
    for (QAction *act : actions) {

        int tempType = act->data().toInt();

        // IconSize checked
        if (tempType == customActionType.key(DesktopCustomAction::kIconSize)) {
            QList<QAction *> subActLst = act->menu()->actions();
            int i = 0;
            for (QAction *tempAct : subActLst) {
                tempAct->setCheckable(true);
                tempAct->setChecked(i == view->itemDelegate()->iconLevel());
                i++;
            }
        }

        // paste enable
        bool clipBoardUnknow = ClipBoard::instance()->clipboardAction() == ClipBoard::kUnknownAction;
        if (tempType == kActPaste)
            act->setEnabled(clipBoardUnknow ? false : true);

        // auto arrange checked
        if (tempType == customActionType.key(DesktopCustomAction::kAutoArrange)) {
            act->setCheckable(true);
            auto align = DispalyIns->autoAlign();
            act->setChecked(align);
        }
    }
}

bool CanvasMenu::isRefreshOn() const
{
    // the gsetting control for refresh action
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.contextmenu")) {
        static const QGSettings menuSwitch("com.deepin.dde.filemanager.contextmenu",
                                           "/com/deepin/dde/filemanager/contextmenu/");
        if (menuSwitch.keys().contains("refresh")) {
            auto showRefreh = menuSwitch.get("refresh");
            if (showRefreh.isValid())
                return showRefreh.toBool();
        }
    }

    return false;
}
