// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultentryfileentity.h"
#include "vaulthelper.h"
#include "pathmanager.h"

#include <QApplication>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultEntryFileEntity::VaultEntryFileEntity(QObject *parent)
    : QObject(parent)
{
    fileCalculationUtils = new FileStatisticsJob;
    connect(fileCalculationUtils, &FileStatisticsJob::dataNotify, this, &VaultEntryFileEntity::slotFileDirSizeChange);
    connect(fileCalculationUtils, &FileStatisticsJob::finished, this, &VaultEntryFileEntity::slotFinishedThread);
    fileCalculationUtils->setFileHints(FileStatisticsJob::FileHint::kNoFollowSymlink | FileStatisticsJob::FileHint::kDontSizeInfoPointer);
}

VaultEntryFileEntity::~VaultEntryFileEntity()
{
    fileCalculationUtils->stop();
    fileCalculationUtils->deleteLater();
}

QString VaultEntryFileEntity::displayName() const
{
    return tr("File Vault");
}

QIcon VaultEntryFileEntity::icon() const
{
    return QIcon::fromTheme("safebox");
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
    if (VaultHelper::instance()->state(PathManager::vaultLockPath()) == VaultState::kUnlocked) {
        fmDebug() << "Vault: Vault is unlocked, starting size calculation";
        showSizeState = true;
        QUrl url = VaultHelper::instance()->vaultToLocalUrl(VaultHelper::instance()->sourceRootUrl());
        fileCalculationUtils->start(QList<QUrl>() << url);
        return true;
    }
    fmDebug() << "Vault: Vault is not unlocked, skipping size calculation";
    return false;
}

bool VaultEntryFileEntity::showUsageSize() const
{
    return false;
}

AbstractEntryFileEntity::EntryOrder VaultEntryFileEntity::order() const
{
    AbstractEntryFileEntity::EntryOrder order = AbstractEntryFileEntity::EntryOrder(static_cast<int>(AbstractEntryFileEntity::EntryOrder::kOrderCustom) + 1);
    return order;
}

void VaultEntryFileEntity::refresh()
{
}

quint64 VaultEntryFileEntity::sizeTotal() const
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

void VaultEntryFileEntity::slotFileDirSizeChange(qint64 size, int filesCount, int directoryCount)
{
    Q_UNUSED(filesCount)
    Q_UNUSED(directoryCount)
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
