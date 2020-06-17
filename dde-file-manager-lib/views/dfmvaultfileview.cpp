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
#include "dfmvaultfileview.h"
#include "controllers/vaultcontroller.h"
#include "vault/vaultlockmanager.h"

#include "views/dfmvaultunlockpages.h"
#include "views/dfmvaultrecoverykeypages.h"
#include "views/dfmvaultremovepages.h"
#include "views/dfmvaultactiveview.h"
#include "views/dfilemanagerwindow.h"

DWIDGET_USE_NAMESPACE

DFMVaultFileView::DFMVaultFileView(QWidget *parent)
    : DFileView(parent)
{
}

bool DFMVaultFileView::setRootUrl(const DUrl &url)
{
    VaultController::VaultState enState = VaultController::getVaultController()->state();

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
        default:;
        }
    } else {
        if (url.host() == "delete") {
            page = DFMVaultRemovePages::instance();
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
