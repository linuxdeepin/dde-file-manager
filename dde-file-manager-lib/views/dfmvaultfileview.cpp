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



#include <QHBoxLayout>
#include <QLabel>
#include <DIconButton>
#include <QMenu>

DWIDGET_USE_NAMESPACE

VaultHeaderView::VaultHeaderView(QWidget *parent)
    : QWidget (parent)
{
    QLabel * lb = new QLabel(tr("File Vault"), this);
    QFont font = lb->font();
    font.setBold(true);
    font.setPixelSize(24);
    lb->setFont(font);

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(lb);    
    layout->addStretch();
}

DFileMenu *VaultHeaderView::createMenu()
{
    DFileMenu * menu = new DFileMenu;

    menu->addAction(tr("Lock vault"), this, &VaultHeaderView::requestLockVault);
    menu->addAction(tr("Generate key"), this, &VaultHeaderView::requestGenerateRecoveryKey);

    return menu;
}

// --------------------------------------------

DFMVaultFileView::DFMVaultFileView(QWidget *parent)
    : DFileView(parent)
{
    connect(VaultController::getVaultController(), &VaultController::signalLockVault, this, &DFMVaultFileView::lockVault);

    connect(VaultController::getVaultController(), &VaultController::vaultRepaint, this, &DFMVaultFileView::onVaultRepaint);
}

bool DFMVaultFileView::setRootUrl(const DUrl &url)
{
    VaultController::VaultState state = VaultController::getVaultController()->state();

    if (state != VaultController::Unlocked) {
        switch (state) {
        case VaultController::NotExisted:
            return false;
        case VaultController::Encrypted:
            return false;
        default:
            return false;
        }
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

void DFMVaultFileView::lockVault(int state)
{
    if(state == 0)
    {
        DUrl url(COMPUTER_ROOT);
        cd(url);
    }
}

void DFMVaultFileView::onVaultRepaint()
{
    DFMVaultFileView *w = (DFMVaultFileView*)this;
    w->repaint();
}
