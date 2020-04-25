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

    DIconButton * menuBtn = new DIconButton(DStyle::SP_TitleBarMenuButton, this);
    menuBtn->setFixedSize(QSize(32, 32));

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(lb);
    layout->addStretch();

    layout->addWidget(menuBtn);

    connect(menuBtn, &QAbstractButton::clicked, this, [=](){
        DFileMenu * menu = createMenu();
        menu->exec(menuBtn->mapToGlobal(menuBtn->rect().center()));
        menu->deleteLater();
    });
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
    VaultHeaderView * headerView = new VaultHeaderView(this);
    int index = this->addHeaderWidget(headerView);
    Q_UNUSED(index);

    connect(headerView, &VaultHeaderView::requestLockVault, this, [this](){
        if (VaultController::lockVault()) {
            cd(VaultController::makeVaultUrl("/", "unlock"));
        }
    });

    connect(headerView, &VaultHeaderView::requestGenerateRecoveryKey, this, [this](){
        cd(VaultController::makeVaultUrl("/verify", "recovery_key"));
    });
}

bool DFMVaultFileView::setRootUrl(const DUrl &url)
{
    VaultController::VaultState state = VaultController::state();

    if (state != VaultController::Unlocked) {
        switch (state) {
        case VaultController::NotExisted:
            cd(VaultController::makeVaultUrl("/", "setup"));
            return false;
        case VaultController::Encrypted:
            cd(VaultController::makeVaultUrl("/", "unlock"));
            return false;
        default:
            return false;
        }
    }

    return DFileView::setRootUrl(url);
}
