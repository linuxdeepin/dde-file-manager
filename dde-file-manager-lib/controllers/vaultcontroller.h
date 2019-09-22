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
#pragma once

#include "dabstractfilecontroller.h"

#include <DSecureString>

DCORE_USE_NAMESPACE

class VaultController : public DAbstractFileController
{
    Q_OBJECT
public:
    enum VaultState {
        NotExisted,
        Encrypted,
        Unlocked,
        UnderProcess,
        Broken,
        NotAvailable
    };

    explicit VaultController(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;

    static void prepareVaultDirs();
    static bool runVaultProcess(QStringList arguments, const DSecureString &stdinString);
    static DUrl makeVaultUrl(QString path = "", QString host = "files");
    static QString makeVaultLocalPath(QString path = "", QString base = "vault_unlocked");
    static DUrl localUrlToVault(const DUrl &vaultUrl);
    static DUrl localToVault(QString localPath);
    static QString vaultToLocal(const DUrl &vaultUrl);
    static DUrl vaultToLocalUrl(const DUrl &vaultUrl);
    static VaultState state();

    static bool createVault(const DSecureString &password);
    static bool unlockVault(const DSecureString &password);
    static bool lockVault();
};
