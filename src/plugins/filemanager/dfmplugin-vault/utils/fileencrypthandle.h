// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHANDLE_H
#define VAULTHANDLE_H

#include "dfmplugin_vault_global.h"

#include <QObject>
#include <QDBusMessage>

namespace dfmplugin_vault {
class FileEncryptHandlerPrivate;
class FileEncryptHandle : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileEncryptHandle)

public:
    static FileEncryptHandle *instance();

    void createVault(const QString &lockBaseDir, const QString &unlockFileDir, const QString &DSecureString,
                     EncryptType type = EncryptType::AES_256_GCM, int blockSize = 32768);
    bool unlockVault(const QString &lockBaseDir, const QString &unlockFileDir, const QString &DSecureString);
    bool lockVault(QString unlockFileDir, bool isForced);
    bool createDirIfNotExist(QString path);
    VaultState state(const QString &encryptBaseDir, bool useCache = true) const;
    bool updateState(VaultState curState);

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
}

#endif   // VAULTHANDLE_H
