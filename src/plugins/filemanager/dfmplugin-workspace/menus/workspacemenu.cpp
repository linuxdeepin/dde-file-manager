/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "workspacemenu.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "utils/fileoperatorhelper.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"

#include "services/common/bluetooth/bluetoothservice.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileactions.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-framework/framework.h"

#include <QMenu>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

WorkspaceMenu::WorkspaceMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *WorkspaceMenu::build(QWidget *parent,
                            AbstractMenu::MenuMode mode,
                            const QUrl &rootUrl,
                            const QUrl &focusUrl,
                            const QList<QUrl> &selected,
                            QVariant customData)
{
    Q_UNUSED(customData)

    view = qobject_cast<FileView *>(parent);
    selectUrls = selected;
    curUrl = focusUrl;

    QMenu *menu = new QMenu(parent);

    switch (mode) {
    case AbstractMenu::MenuMode::kEmpty:
        assemblesEmptyAreaActions(menu, rootUrl);
        break;
    case AbstractMenu::MenuMode::kNormal:
        assemblesNormalActions(menu, rootUrl, focusUrl, selected);
        break;
    }

    return menu;
}

void WorkspaceMenu::actionBusiness(QAction *act)
{
    auto actType = act->data().toInt();

    switch (actType) {
    case kActOpen:
        FileOperatorHelperIns->openFiles(view);
        break;
    case kActOpenInNewWindow:
        WorkspaceEventCaller::sendOpenWindow(view->selectedUrlList());
        break;
    case kActOpenInNewTab: {
        const quint64 winID = WorkspaceHelper::instance()->windowId(view);
        WorkspaceHelper::instance()->actionNewTab(winID, curUrl);
        break;
    }
    case kActSelectAll:
        view->selectAll();
        break;
    case kActDisplayAsIcon:
        view->setViewMode(Global::ViewMode::kIconMode);
        break;
    case kActDisplayAsList:
        view->setViewMode(Global::ViewMode::kListMode);
        break;
    case kActName:
    case kActSize:
    case kActType:
    case kActLastModifiedDate: {
        FileViewItem::Roles role = static_cast<FileViewItem::Roles>(getRoleByActionType(static_cast<ActionType>(actType)));
        Qt::SortOrder order = view->proxyModel()->sortOrder();
        int column = view->model()->getColumnByRole(role);

        view->proxyModel()->setSortRole(role);
        view->proxyModel()->sort(column, order);

        break;
    }
    case kActNewFolder:
        FileOperatorHelperIns->touchFolder(view);
        break;
    case kActNewText:
        FileOperatorHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeText);
        break;
    case kActNewWord:
        FileOperatorHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeWord);
        break;
    case kActNewExcel:
        FileOperatorHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeExcel);
        break;
    case kActNewPowerpoint:
        FileOperatorHelperIns->touchFiles(view, CreateFileType::kCreateFileTypePowerpoint);
        break;
    case kActCut:
        FileOperatorHelperIns->cutFiles(view);
        break;
    case kActCopy:
        FileOperatorHelperIns->copyFiles(view);
        break;
    case kActPaste:
        FileOperatorHelperIns->pasteFiles(view);
        break;
    case kActDelete:
        FileOperatorHelperIns->moveToTrash(view);
        break;
    case kActRename:
        if (selectUrls.count() > 1) {
            const quint64 winID = WorkspaceHelper::instance()->windowId(view);
            WorkspaceEventCaller::sendShowCustomTopWidget(winID, SchemeTypes::kFile, true);
        } else {
            const QModelIndex &index = view->selectionModel()->currentIndex();
            if (index.isValid())
                view->edit(index);
        }
        break;
    case kActSendToDesktop: {
        QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
        FileOperatorHelperIns->createSymlink(view, StandardPaths::toStandardUrl(desktopPath));
        break;
    }
    case kActCreateSymlink:
        FileOperatorHelperIns->createSymlink(view);
        break;
    case kActOpenInTerminal:
        FileOperatorHelperIns->openInTerminal(view);
        break;
    case kActProperty:
        FileOperatorHelperIns->showFilesProperty(view);
        break;
    case ActionType::kActSendToBluetooth:
        FileOperatorHelperIns->sendBluetoothFiles(view);
        break;
    default:
        break;
    }

    if (sendToRemovabalDiskActs.contains(actType)) {
        const QUrl &target = sendToRemovabalDiskActs.value(actType);
        // TODO(xust): publish copy event.
        qDebug() << "send files to: " << target;
    }
}

void WorkspaceMenu::assemblesEmptyAreaActions(QMenu *menu, const QUrl &rootUrl)
{
    QString errString;
    AbstractFileInfoPointer fileInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(rootUrl, true, &errString);
    if (!fileInfo) {
        qWarning() << "create LocalFileInfo error: " << errString;
        return;
    }

    QVector<ActionType> typeList;

    AbstractFileActions *actionInfo = new AbstractFileActions(fileInfo);
    typeList = actionInfo->menuActionList(AbtMenuType::kSpaceArea);

    QVector<ActionDataContainer> dataList;

    transTypesToActionsData(typeList, dataList);
    assemblesSubActions(dataList);

    addActionsToMenu(menu, dataList);
}

void WorkspaceMenu::assemblesNormalActions(QMenu *menu, const QUrl &rootUrl, const QUrl &focusUrl, const QList<QUrl> &selectList)
{
    QString errString;
    AbstractFileInfoPointer fileInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(focusUrl, true, &errString);
    if (!fileInfo) {
        qWarning() << "create LocalFileInfo error: " << errString;
        return;
    }

    QVector<ActionType> typeList;

    AbstractFileActions *actionInfo = new AbstractFileActions(fileInfo);

    if (selectList.count() > 1) {
        typeList = actionInfo->menuActionList(AbtMenuType::kMultiFiles);
    } else {
        typeList = actionInfo->menuActionList(AbtMenuType::kSingleFile);
    }

    QVector<ActionDataContainer> dataList;

    transTypesToActionsData(typeList, dataList);
    assemblesSubActions(dataList);

    bool hasFolder = actionInfo->isDir();
    if (!hasFolder) {
        for (const QUrl &url : selectList) {
            auto info = dfmbase::InfoFactory::create<AbstractFileInfo>(url, true);
            hasFolder = info && info->isDir();
            if (hasFolder)
                break;
        }
    }
    auto sendToActs = getSendToActions(hasFolder);
    if (!sendToActs.childrenActionsData().isEmpty())
        dataList.append(sendToActs);

    addActionsToMenu(menu, dataList);
}

void WorkspaceMenu::addActionsToMenu(QMenu *menu, const QVector<ActionDataContainer> &dataList)
{
    if (dataList.isEmpty())
        return;

    // add action to menu
    for (const ActionDataContainer &data : dataList) {
        // Separator
        if (data.actionType() == ActionType::kActSeparator) {
            menu->addSeparator();
            continue;
        }

        if (data.name().isEmpty() || (-1 == data.actionType()))
            continue;

        QAction *act = new QAction(menu);
        act->setData(data.actionType());
        act->setText(data.name());
        if (!data.icon().isNull())
            act->setIcon(data.icon());

        QVector<ActionDataContainer> subDataList = data.childrenActionsData();
        if (!subDataList.isEmpty()) {
            QMenu *subMenu = new QMenu(menu);
            addActionsToMenu(subMenu, subDataList);
            act->setMenu(subMenu);
        }

        menu->addAction(act);
    }
}

void WorkspaceMenu::transTypesToActionsData(const QVector<ActionType> &typeList, QVector<ActionDataContainer> &dataList)
{
    for (const ActionType &type : typeList) {
        dataList.append(ActionTypeManager::instance().actionDataContainerByType(type));
    }
}

void WorkspaceMenu::assemblesSubActions(QVector<ActionDataContainer> &dataList)
{
    for (ActionDataContainer &data : dataList) {
        if (data.actionType() == ActionType::kActDisplayAs) {
            // TODO(liuyangming): check support view mode
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActDisplayAsList));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActDisplayAsIcon));
        }

        if (data.actionType() == ActionType::kActSortBy) {
            // TODO(liuyangming): get support sort
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActName));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActLastModifiedDate));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSize));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActType));
        }
    }
}

ActionDataContainer WorkspaceMenu::getSendToActions(bool hasFolder)
{
    ActionDataContainer actSendTo = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSendToRemovableDisk);
    QVector<ActionDataContainer> subActs;

    DSC_USE_NAMESPACE
    if (BluetoothService::service() && BluetoothService::service()->bluetoothEnable()) {
        auto act = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSendToBluetooth);
        if (hasFolder) {
            // TODO(xust) disable the action
        }
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

int WorkspaceMenu::getRoleByActionType(const ActionType type) const
{
    switch (type) {
    case ActionType::kActName:
        return FileViewItem::kItemNameRole;
    case ActionType::kActLastModifiedDate:
        return FileViewItem::kItemFileLastModifiedRole;
    case ActionType::kActSize:
        return FileViewItem::kItemFileSizeRole;
    case ActionType::kActType:
        return FileViewItem::kItemFileMimeTypeRole;
    default:
        return FileViewItem::kItemNameRole;
    }
}
