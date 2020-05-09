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
    DFileMenu *menu = new DFileMenu();

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    VaultController::VaultState vaultState = VaultController::state();

    if (vaultState == VaultController::Unlocked) {
        // 打开
        menu->addAction(QObject::tr("Open"), [wnd, item]() {
            wnd->cd(item->url());
        })->setDisabled(shouldDisable);

        // 在新窗口打开
        menu->addAction(QObject::tr("Open in new window"), [item]() {
            WindowManager::instance()->showNewWindow(item->url(), true);
        });

        menu->addSeparator();

        // 立即上锁
        menu->addAction(QObject::tr("Lock"), [this]() {
            lockNow();
        });

        // 自动上锁
        QAction *autoLockAction = menu->addAction(QObject::tr("Auto lock"));

        DFileMenu *autoLockMenu = new DFileMenu();
        autoLockMenu->addAction(QObject::tr("Never"), [this]() {
            autoLock(0);
        })->setCheckable(true);

        autoLockMenu->addSeparator();

        autoLockMenu->addAction(QObject::tr("5 minutes"), [this]() {
            autoLock(5);
        })->setCheckable(true);

        autoLockMenu->addAction(QObject::tr("10 minutes"), [this]() {
            autoLock(10);
        })->setCheckable(true);

        autoLockMenu->addAction(QObject::tr("20 minutes"), [this]() {
            autoLock(20);
        })->setCheckable(true);

        autoLockMenu->QObject::setParent(menu);
        autoLockAction->setMenu(autoLockMenu);

        menu->addSeparator();

        // 删除保险箱
        menu->addAction(QObject::tr("Remove File Vault"), [this]() {
            showDeleteVaultView();
        });

        menu->addSeparator();

        // 显示属性对话框
        menu->addAction(QObject::tr("Properties"), [item]() {
            DUrlList list;
            list.append(item->url());
            Singleton<FileSignalManager>::instance()->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, list));
        });
    } else if (vaultState == VaultController::Encrypted) {
        // 解锁
        menu->addAction(QObject::tr("Unlock"), [this]() {
            showUnLockView();
        })->setCheckable(true);

        //使用恢复凭证
        menu->addAction(QObject::tr("Unlock by key"), [this]() {
            showCertificateView();
        })->setCheckable(true);
    }

    return menu;
}

bool DFMSideBarVaultItemHandler::lockNow()
{
    // Something to do.
    return VaultController::lockVault();
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
