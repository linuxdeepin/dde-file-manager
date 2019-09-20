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
#include "dfmvaultunlockpages.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <DFloatingButton>
#include "dpasswordedit.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMVaultUnlockPages::DFMVaultUnlockPages(QWidget *parent)
    : DFMVaultContentInterface(parent)
{
    QLabel * description = new QLabel("Sample text, sample text.\nSample text.", this);
    description->setAlignment(Qt::AlignHCenter);

    DFloatingButton * unlockButton = new DFloatingButton(DStyle::SP_UnlockElement, this);
    DPasswordEdit * password = new DPasswordEdit(this);

    QPushButton * icon = new QPushButton(this);
    icon->setDisabled(true);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setMinimumHeight(64);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(description);
    layout->addWidget(password);
    layout->addWidget(unlockButton);
    layout->addStretch();

    layout->setAlignment(unlockButton, Qt::AlignHCenter);
}

QPair<DUrl, bool> DFMVaultUnlockPages::requireRedirect(VaultController::VaultState state)
{
    switch (state) {
    case VaultController::NotExisted:
        return {VaultController::makeVaultUrl("/", "setup"), true};
    case VaultController::Unlocked:
        return {VaultController::makeVaultUrl(), true};
    default:
        break;
    }
    return DFMVaultContentInterface::requireRedirect(state);
}

DFM_END_NAMESPACE
