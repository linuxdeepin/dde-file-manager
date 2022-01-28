/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaultentryfileentity.h"
#include "vaulthelper.h"

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultEntryFileEntity::VaultEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    vaultCalculationUtils = new VaultCalculationUtils;
    connect(vaultCalculationUtils, &VaultCalculationUtils::sigTotalChange, this, &VaultEntryFileEntity::slotFileDirSizeChange);
}

VaultEntryFileEntity::~VaultEntryFileEntity()
{
    vaultCalculationUtils->deleteLater();
}

QString VaultEntryFileEntity::displayName() const
{
    return tr("My Vault");
}

QIcon VaultEntryFileEntity::icon() const
{
    return QIcon::fromTheme("dfm_safebox");
}

bool VaultEntryFileEntity::exists() const
{
    return true;
}

bool VaultEntryFileEntity::showProgress() const
{
    return false;
}

bool VaultEntryFileEntity::showTotalSize() const
{
    if (VaultHelper::state(VaultHelper::vaultLockPath()) == VaultState::kUnlocked) {
        vaultCalculationUtils->startThread(QList<QUrl>() << VaultHelper::rootUrl());
        return true;
    }
    return false;
}

bool VaultEntryFileEntity::showUsageSize() const
{
    return false;
}

void VaultEntryFileEntity::onOpen()
{
}

EntryFileInfo::EntryOrder VaultEntryFileEntity::order() const
{
    EntryFileInfo::EntryOrder order = EntryFileInfo::EntryOrder(static_cast<int>(EntryFileInfo::EntryOrder::kOrderCustom) + 1);
    return order;
}

QMenu *VaultEntryFileEntity::createMenu()
{
    return VaultHelper::createMenu();
}

void VaultEntryFileEntity::refresh()
{
}

qint64 VaultEntryFileEntity::sizeTotal() const
{
    return vaultTotal;
}

QUrl VaultEntryFileEntity::targetUrl() const
{
    QUrl url;
    url.setScheme(VaultHelper::scheme());
    url.setHost("");
    url.setPath("/");
    return url;
}

void VaultEntryFileEntity::slotFileDirSizeChange(qint64 size)
{
    vaultTotal = size;
}
