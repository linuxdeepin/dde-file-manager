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

#include "dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "vault/vaultlockmanager.h"
#include "dfmsettings.h"
#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultremovepages.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfilemanagerwindow.h"
#include "dfilesystemmodel.h"
#include "vaultglobaldefine.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"

#include <QDrag>

DFMVaultFileView::DFMVaultFileView(QWidget *parent)
    : DFileView(parent)
{
    connect(VaultController::ins(), &VaultController::signalLockVault, this, &DFMVaultFileView::onLeaveVault);
    connect(fileSignalManager, &FileSignalManager::requestIgnoreDragEvent, this, &DFMVaultFileView::IgnoreDragEvent);
}

bool DFMVaultFileView::setRootUrl(const DUrl &url)
{
    if(!VaultController::ins()->isVaultVisiable()) {
        DDialog dialog(tr("Hint"), tr("Cannot open this path"), this);
        dialog.addButton(tr("OK"));
        dialog.exec();
        return false;
    }

    VaultController::VaultState enState = VaultController::ins()->state();

    QWidget *wndPtr = widget()->topLevelWidget();
    DFMVaultPageBase *page = nullptr;
    if (enState != VaultController::Unlocked) {
        switch (enState) {
        case VaultController::NotAvailable: {
            qDebug() << "cryfs not installed!";
            break;
        }
        case VaultController::NotExisted: {
            page = DFMVaultActiveView::getInstance();
            break;
        }
        case VaultController::Encrypted: {
            if (url.host() == "certificate") {
                page = DFMVaultRecoveryKeyPages::instance();
            } else {
                page = DFMVaultUnlockPages::instance();
            }
            break;
        }
        default:
            ;
        }
    } else {
        if (url.host() == "delete") {
            page = DFMVaultRemovePages::instance();
        }

        //! 记录访问保险箱时间
        if (VaultController::isRootDirectory(url.toLocalFile())) {
            VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::VAULTPAGE);
            DFM_NAMESPACE::DFMSettings setting(VAULT_TIME_CONFIG_FILE);
            setting.setValue(QString("VaultTime"), QString("InterviewTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        }
    }

    if (page) {
        page->setWndPtr(wndPtr);
        page->showTop();
        return false;
    }

    if (DFMVaultRemovePages::instance()->isVisible()) {
        DFMVaultRemovePages::instance()->raise();
    }

    return DFileView::setRootUrl(url);
}

void DFMVaultFileView::onLeaveVault(int state)
{
    if (state == 0) {
        this->cd(DUrl(COMPUTER_ROOT));
        VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
    }
}

void DFMVaultFileView::IgnoreDragEvent()
{
    QDrag::cancel();
}

bool DFMVaultFileView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::HoverMove
            || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::KeyRelease) {

        VaultLockManager::getInstance().refreshAccessTime();
#ifdef AUTOLOCK_TEST
        qDebug() << "type == " << event->type();
#endif
    }

    return DFileView::eventFilter(obj, event);
}

void DFMVaultFileView::onRootUrlDeleted(const DUrl &rootUrl)
{
    // 修复bug-77747 保险箱上锁情况下会跳转到计算机页面，所以这里不用执行跳转
    if (VaultController::ins()->state() == VaultController::Encrypted)
        return;

    DFileView::onRootUrlDeleted(rootUrl);
}
