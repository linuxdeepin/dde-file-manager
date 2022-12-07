/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#ifndef VAULTHANDLE_H
#define VAULTHANDLE_H

#include "dfmplugin_vault_global.h"

#include <QObject>

DPVAULT_BEGIN_NAMESPACE

class FileEncryptHandlerPrivate;
class FileEncryptHandle : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileEncryptHandle)

public:
    static FileEncryptHandle *instance();

    void createVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type = EncryptType::AES_256_GCM, int blockSize = 32768);
    void unlockVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString);
    void lockVault(QString unlockFileDir);
    void createDirIfNotExist(QString path);
    VaultState state(const QString &encryptBaseDir, const QString &decryptFileDir) const;

    EncryptType encryptAlgoTypeOfGroupPolicy();
signals:
    void signalReadError(QString error);
    void signalReadOutput(QString msg);
    void signalCreateVault(int state);
    void signalUnlockVault(int state);
    void signalLockVault(int state);

public slots:
    void slotReadError();
    void slotReadOutput();

private:
    explicit FileEncryptHandle(QObject *parent = nullptr);
    virtual ~FileEncryptHandle() override;

private:
    FileEncryptHandlerPrivate *d = nullptr;
};

DPVAULT_END_NAMESPACE

#endif   // VAULTHANDLE_H
