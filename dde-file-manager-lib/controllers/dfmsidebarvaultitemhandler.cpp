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

#include "vault/interfaceactivevault.h"
#include "vault/vaultlockmanager.h"

#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultremovepages.h"
#include "views/dfmvaultactiveview.h"

#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"

#include <QDialog>


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

    // Initalize vault manager.
    VaultLockManager::getInstance();

    return item;
}

DFMSideBarVaultItemHandler::DFMSideBarVaultItemHandler(QObject *parent)
    : DFMSideBarItemInterface(parent)
{

}

void DFMSideBarVaultItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    EN_VaultState enState = InterfaceActiveVault::vaultState();
    switch (enState) {
    case EN_VaultState::NotAvailable:{  // 没有安装cryfs
        qDebug() << "Don't setup cryfs, can't use vault, please setup cryfs!";
        break;
    }
    case EN_VaultState::NotExisted:{    // 没有创建过保险箱，此时创建保险箱,创建成功后，进入主界面
        DFMVaultActiveView::getInstance().showTop();
        break;
    }
    case EN_VaultState::Encrypted:{ // 保险箱处于加密状态，弹出开锁对话框,开锁成功后，进入主界面
        // todo
        if(QDialog::Accepted == DFMVaultUnlockPages::instance()->exec()){
            // 进入保险箱
            DFMSideBarItemInterface::cdAction(sidebar, item);
        }
        break;
    }
    case EN_VaultState::Unlocked:{  // 保险箱处于开锁状态，直接进入主界面
        DFMSideBarItemInterface::cdAction(sidebar, item);
        break;
    }
    default:{   // 未考虑
        break;
    }
    }
}

QMenu *DFMSideBarVaultItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    Q_UNUSED(item)
    return generateMenu(sidebar->topLevelWidget(), sidebar);
}

DFileMenu *DFMSideBarVaultItemHandler::generateMenu(QWidget *topWidget, const DFMSideBar *sender)
{
    DFileMenu *menu = nullptr;

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topWidget);
    VaultController *controller = VaultController::getVaultController();

    VaultController::VaultState vaultState = controller->state();

    DUrl url = controller->makeVaultUrl();
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, url);

    QSet<MenuAction> disableList;
    if (!VaultLockManager::getInstance().isValid()) {
        disableList << MenuAction::FiveMinutes
                    << MenuAction::TenMinutes
                    << MenuAction::TwentyMinutes;
    }

    menu = DFileMenuManager::genereteMenuByKeys(infoPointer->menuActionList(), disableList, true, infoPointer->subMenuActionList(), false);
    menu->setEventData(DUrl(), {url}, WindowManager::getWindowId(wnd), sender);

    if (vaultState == VaultController::Unlocked) {

        // 立即上锁
        QAction *action = DFileMenuManager::getAction(MenuAction::LockNow);
        QObject::connect(action, &QAction::triggered, action, [this, wnd](){
            lockNow(wnd);
        });

        // 自动上锁
        VaultLockManager::AutoLockState lockState = VaultLockManager::getInstance().autoLockState();

        QAction *actionNever = DFileMenuManager::getAction(MenuAction::Never);
        QObject::connect(actionNever, &QAction::triggered, actionNever, [this](){
            autoLock(VaultLockManager::Never);
        });
        actionNever->setCheckable(true);
        actionNever->setChecked(lockState == VaultLockManager::Never ? true : false);

        QAction *actionFiveMins = DFileMenuManager::getAction(MenuAction::FiveMinutes);
        QObject::connect(actionFiveMins, &QAction::triggered, actionFiveMins, [this](){
            autoLock(VaultLockManager::FiveMinutes);
        });
        actionFiveMins->setCheckable(true);
        actionFiveMins->setChecked(lockState == VaultLockManager::FiveMinutes ? true : false);

        QAction *actionTenMins = DFileMenuManager::getAction(MenuAction::TenMinutes);
        QObject::connect(actionTenMins, &QAction::triggered, actionTenMins, [this](){
            autoLock(VaultLockManager::TenMinutes);
        });
        actionTenMins->setCheckable(true);
        actionTenMins->setChecked(lockState == VaultLockManager::TenMinutes ? true : false);

        QAction *actionTwentyMins = DFileMenuManager::getAction(MenuAction::TwentyMinutes);
        QObject::connect(actionTwentyMins, &QAction::triggered, actionTwentyMins, [this](){
            autoLock(VaultLockManager::TwentyMinutes);
        });
        actionTwentyMins->setCheckable(true);
        actionTwentyMins->setChecked(lockState == VaultLockManager::TwentyMinutes ? true : false);

        // 删除保险柜
        action = DFileMenuManager::getAction(MenuAction::DeleteVault);
        QObject::connect(action, &QAction::triggered, action, [this, wnd](){
            showDeleteVaultView(wnd);
        });
    } else if (vaultState == VaultController::Encrypted) {

        // 解锁
        QAction *action = DFileMenuManager::getAction(MenuAction::UnLock);
        QObject::connect(action, &QAction::triggered, action, [this, wnd](){
            showUnLockView(wnd);
        });

        // 使用恢复凭证
        action = DFileMenuManager::getAction(MenuAction::UnLockByKey);
        QObject::connect(action, &QAction::triggered, action, [this, wnd](){
            showCertificateView(wnd);
        });
    }

    return menu;
}

bool DFMSideBarVaultItemHandler::lockNow(DFileManagerWindow *wnd)
{
    // 如果正在有保险箱的移动、粘贴、删除操作，置顶弹出任务框
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if(pTaskDlg){
        if(pTaskDlg->bHaveNotCompletedVaultTask()){
            // Flashing alert
            pTaskDlg->hide();
            pTaskDlg->showDialogOnTop();
        }
    }

    // 关闭当前的标签页面
    emit fileSignalManager->requestCloseAllTabOfVault(wnd->windowId());

    // 保险箱上锁
    VaultController::getVaultController()->lockVault();
    return true;
}

bool DFMSideBarVaultItemHandler::autoLock(int lockState)
{
    return VaultLockManager::getInstance().autoLock(static_cast<VaultLockManager::AutoLockState>(lockState));
}

void DFMSideBarVaultItemHandler::showDeleteVaultView(DFileManagerWindow *wnd)
{
    // Something to do.
    if(DDialog::Accepted == DFMVaultRemovePages::instance()->exec()){
        // 切换到home目录下
        wnd->cd(DUrl(COMPUTER_ROOT));
    }
}

void DFMSideBarVaultItemHandler::showUnLockView(DFileManagerWindow *wnd)
{
    // Something to do.
    if(QDialog::Accepted == DFMVaultUnlockPages::instance()->exec()){
        wnd->cd(VaultController::getVaultController()->makeVaultUrl());
    }
}

void DFMSideBarVaultItemHandler::showCertificateView(DFileManagerWindow *wnd)
{
    // Something to do.
    if(QDialog::Accepted == DFMVaultRecoveryKeyPages::instance()->exec()){
        wnd->cd(VaultController::getVaultController()->makeVaultUrl());
    }
}

DFM_END_NAMESPACE
