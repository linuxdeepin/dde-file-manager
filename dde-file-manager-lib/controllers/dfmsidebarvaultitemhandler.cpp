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

#include <DWindowManagerHelper>
#include <DForeignWindow>

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
                VaultController::makeVaultUrl(VaultController::makeVaultLocalPath())
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
        DFMVaultActiveView::getInstance().setWndPtr(sidebar->topLevelWidget());
        DFMVaultActiveView::getInstance().showTop();
        break;
    }
    case EN_VaultState::Encrypted:{ // 保险箱处于加密状态，弹出开锁对话框,开锁成功后，进入主界面
        // todo
        showUnLockView(sidebar->topLevelWidget());
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

    DUrl url = controller->vaultToLocalUrl(controller->makeVaultUrl());
    url.setScheme(DFMVAULT_SCHEME);
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
        QObject::connect(action, &QAction::triggered, action, [this, topWidget](){
            showDeleteVaultView(topWidget);
        });        
    } else if (vaultState == VaultController::Encrypted) {

        // 解锁
        QAction *action = DFileMenuManager::getAction(MenuAction::UnLock);
        QObject::connect(action, &QAction::triggered, action, [this, topWidget](){
            showUnLockView(topWidget);
        });

        // 使用恢复凭证
        action = DFileMenuManager::getAction(MenuAction::UnLockByKey);
        QObject::connect(action, &QAction::triggered, action, [this, topWidget](){
            showCertificateView(topWidget);
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
            return false;
        }
    }
    // 如果当前有保险箱的压缩或解压缩任务，激活任务对话框进程
    QString strPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString strCmd = QString("ps -xo pid,cmd | grep /usr/bin/deepin-compressor | grep ")
            + strPath
            + QString(" | grep -v grep | awk '{print $1}'");
    QStringList lstShellOutput;
    // 执行shell命令，获得压缩进程PID
    int res = InterfaceActiveVault::executionShellCommand(strCmd, lstShellOutput);
    if(res == 0){   // shell命令执行成功
        QStringList::const_iterator itr = lstShellOutput.begin();
        QSet<QString> setResult;
        for(; itr != lstShellOutput.end(); ++itr){
            setResult.insert(*itr);
        }
        if(setResult.count() > 0){  // 有压缩任务
            // 遍历桌面窗口
            bool bFlag = false;
            for(auto window: DWindowManagerHelper::instance()->currentWorkspaceWindows()) {
                QString strWid = QString("%1").arg(window->pid());
                // 如果当前窗口的进程PID属于压缩进程，则将窗口置顶
                if(setResult.contains(strWid)){
                    window->raise();
                    bFlag = true;
                }
            }
            if(bFlag){
                return false;
            }
        }
    }else{
        qDebug() << "执行查找进程PID命令失败!";
    }

    // 如果正在有保险箱的移动、粘贴到桌面的任务，通知桌面进程置顶任务对话框
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                           "/org/freedesktop/FileManager1",
                                                           "org.freedesktop.FileManager1",
                                                           "topTaskDialog");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if(response.type() == QDBusMessage::ReplyMessage){
        bool bValue = response.arguments().takeFirst().toBool();
        if(bValue){
            return false;
        }
    }else{
        qDebug() << "vault show top taskdialog failed!";
    }

    // 关闭当前的保险箱标签页面
    emit fileSignalManager->requestCloseAllTabOfVault(wnd->windowId());

    // 保险箱上锁
    VaultController::getVaultController()->lockVault();

    return true;
}

bool DFMSideBarVaultItemHandler::autoLock(int lockState)
{
    return VaultLockManager::getInstance().autoLock(static_cast<VaultLockManager::AutoLockState>(lockState));
}

void DFMSideBarVaultItemHandler::showDeleteVaultView(QWidget *wndPtr)
{
    DFMVaultRemovePages::instance()->setWndPtr(wndPtr);
    DFMVaultRemovePages::instance()->showTop();
}

void DFMSideBarVaultItemHandler::showUnLockView(QWidget *wndPtr)
{
    DFMVaultUnlockPages::instance()->setWndPtr(wndPtr);

    DFMVaultUnlockPages::instance()->show();
    DFMVaultUnlockPages::instance()->raise();
}

void DFMSideBarVaultItemHandler::showCertificateView(QWidget *wndPtr)
{
    DFMVaultRecoveryKeyPages::instance()->setWndPtr(wndPtr);

    DFMVaultRecoveryKeyPages::instance()->show();
    DFMVaultRecoveryKeyPages::instance()->raise();
}

DFM_END_NAMESPACE
