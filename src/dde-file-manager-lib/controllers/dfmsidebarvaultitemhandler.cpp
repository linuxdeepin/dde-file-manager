/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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




#include "dfmsidebarvaultitemhandler.h"

#include "dfmsidebaritem.h"

#include "dfilemenumanager.h"

#include "singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "controllers/pathmanager.h"
#include "controllers/vaultcontroller.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "durl.h"
#include "interfaces/dfilemenu.h"

#include "vault/vaultlockmanager.h"
#include "vault/vaulthelper.h"

DFM_BEGIN_NAMESPACE

DFMSideBarItem *DFMSideBarVaultItemHandler::createItem(const QString &pathKey)
{
    QString iconName = systemPathManager->getSystemPathIconName(pathKey);
    if (!iconName.contains("-symbolic")) {
        iconName.append("-symbolic");
    }

    QString pathStr = systemPathManager->getSystemPath(pathKey);

    DFMSideBarItem *item = new DFMSideBarItem(
                QIcon::fromTheme(iconName),
                systemPathManager->getSystemPathDisplayName(pathKey),
                VaultController::makeVaultUrl(VaultController::makeVaultLocalPath())
                );

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled);
    item->setData(SIDEBAR_ID_VAULT, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    //! Initalize vault manager.
    VaultLockManager::getInstance();

    return item;
}

DFMSideBarVaultItemHandler::DFMSideBarVaultItemHandler(QObject *parent)
    : DFMSideBarItemInterface(parent)
{
}

void DFMSideBarVaultItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    DFMSideBarItemInterface::cdAction(sidebar, item);
}

QMenu *DFMSideBarVaultItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    Q_UNUSED(item)
    return generateMenu(sidebar->topLevelWidget(), sidebar);
}

DFileMenu *DFMSideBarVaultItemHandler::generateMenu(QWidget *topWidget, const DFMSideBar *sender)
{
    DFileMenu * menu = DFileMenuManager::createVaultMenu(topWidget, sender);
    if(menu){ menu -> setAccessibleInfo(AC_FILE_MENU_SIDEBAR_VAULT_ITEM);}
    return  menu;
}

bool DFMSideBarVaultItemHandler::lockNow(DFileManagerWindow *wnd)
{
    //! Is there a vault task, top it if exist.
    if(!VaultHelper::topVaultTasks()) {
        emit fileSignalManager->requestCloseAllTabOfVault(wnd->windowId());
        VaultController::ins()->lockVault();
    }

    return true;
}

bool DFMSideBarVaultItemHandler::autoLock(int lockState)
{
    return VaultLockManager::getInstance().autoLock(static_cast<VaultLockManager::AutoLockState>(lockState));
}

void DFMSideBarVaultItemHandler::showView(QWidget *wndPtr, QString host)
{
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(wndPtr);
    wnd->cd(VaultController::makeVaultUrl("/", host));
}

DFM_END_NAMESPACE
