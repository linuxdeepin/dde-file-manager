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

#include <QApplication>

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE
VaultEntryFileEntity::VaultEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::sizeChanged, this, &VaultEntryFileEntity::slotFileDirSizeChange);
    connect(fileCalculationUtils, &FileStatisticsJob::finished, this, &VaultEntryFileEntity::slotFinishedThread);
}

VaultEntryFileEntity::~VaultEntryFileEntity()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
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
    if (VaultHelper::instance()->state(VaultHelper::instance()->vaultLockPath()) == VaultState::kUnlocked) {
        showSizeState = true;
        QUrl url = VaultHelper::instance()->vaultToLocalUrl(VaultHelper::instance()->sourceRootUrl());
        fileCalculationUtils->start(QList<QUrl>() << url);
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
    VaultHelper::instance()->appendWinID(QApplication::activeWindow()->winId());
    return VaultHelper::instance()->createMenu();
}

void VaultEntryFileEntity::refresh()
{
}

qint64 VaultEntryFileEntity::sizeTotal() const
{
    if (vaultTotal > 0)
        return vaultTotal;
    else
        return totalchange;
}

QUrl VaultEntryFileEntity::targetUrl() const
{
    return VaultHelper::instance()->rootUrl();
}

void VaultEntryFileEntity::slotFileDirSizeChange(qint64 size)
{
    if (showSizeState) {
        totalchange = size;
        if (vaultTotal > 0 && totalchange > vaultTotal)
            vaultTotal = totalchange;
    }
}

void VaultEntryFileEntity::slotFinishedThread()
{
    showSizeState = false;
    vaultTotal = totalchange;
}
