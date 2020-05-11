/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "trashmanager.h"
#include "durl.h"
#include "interfaces/dfilemenu.h"

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
        DUrl::fromUserInput(pathStr)
    );

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled);
    item->setData(SIDEBAR_ID_VAULT, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarVaultItemHandler::DFMSideBarVaultItemHandler(QObject *parent)
    : DFMSideBarItemInterface(parent)
{

}

void DFMSideBarVaultItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    return DFMSideBarItemInterface::cdAction(sidebar, item);
}

QMenu *DFMSideBarVaultItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    DFileMenu *menu = nullptr;

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());

    VaultController::VaultState vaultState = VaultController::getVaultController()->state();

    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    QSet<MenuAction> disableList;
    menu = DFileMenuManager::genereteMenuByKeys(infoPointer->menuActionList(), disableList, true, infoPointer->subMenuActionList());
    menu->setEventData(DUrl(), {item->url()}, WindowManager::getWindowId(wnd), sidebar);

    if (vaultState == VaultController::Unlocked) {

        // 立即上锁
        QAction *action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::LockNow));
        QObject::connect(action, &QAction::triggered, action, [this](){
            lockNow();
        });

        // 自动上锁
        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Never));
        QObject::connect(action, &QAction::triggered, action, [this](){
            autoLock(0);
        });

        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::FiveMinutes));
        QObject::connect(action, &QAction::triggered, action, [this](){
            autoLock(5);
        });

        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::TenMinutes));
        QObject::connect(action, &QAction::triggered, action, [this](){
            autoLock(10);
        });

        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::TwentyMinutes));
        QObject::connect(action, &QAction::triggered, action, [this](){
            autoLock(20);
        });

        // 删除保险柜
        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::DeleteVault));
        QObject::connect(action, &QAction::triggered, action, [this](){
            showDeleteVaultView();
        });
    } else if (vaultState == VaultController::Encrypted) {

        // 解锁
        QAction *action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::UnLock));
        QObject::connect(action, &QAction::triggered, action, [this](){
            showUnLockView();
        });

        // 使用恢复凭证
        action = menu->actionAt(DFileMenuManager::getActionString(MenuAction::UnLockByKey));
        QObject::connect(action, &QAction::triggered, action, [this](){
            showCertificateView();
        });
    }

    if (menu == nullptr) {
        menu = new DFileMenu();
    }

    return menu;
}

bool DFMSideBarVaultItemHandler::lockNow()
{
    // Something to do.
    VaultController::getVaultController()->lockVault();
    return false;
}

bool DFMSideBarVaultItemHandler::autoLock(uint minutes)
{
    Q_UNUSED(minutes)
    // Something to do.
    return true;
}

void DFMSideBarVaultItemHandler::showDeleteVaultView()
{
    // Something to do.
}

void DFMSideBarVaultItemHandler::showUnLockView()
{
    // Something to do.
}

void DFMSideBarVaultItemHandler::showCertificateView()
{
    // Something to do.
}

DFM_END_NAMESPACE
