// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masterkeymanager.h"
#include "passwordmanager.h"
#include "utils/vaultdefine.h"

#include "dfmplugin_vault_global.h"

using namespace dfmplugin_vault;

QByteArray MasterKeyManager::generateMasterKey()
{
    const int masterKeySize = 64;
    char masterKeyBuf[masterKeySize];

    int ret = PasswordManager::generateRandomBytes(masterKeyBuf, masterKeySize);
    if (ret != 0) {
        fmCritical() << "Vault MasterKeyManager: Failed to generate master key";
        return QByteArray();
    }

    return QByteArray(masterKeyBuf, masterKeySize);
}

QByteArray MasterKeyManager::generateMasterKeyFromPassword(const QString &password)
{
    QByteArray passwordBytes = password.toUtf8();
    const int masterKeySize = 64;

    // 补零至64字节
    QByteArray masterKey(masterKeySize, '\0');
    int copySize = qMin(passwordBytes.size(), masterKeySize);
    if (copySize > 0) {
        memcpy(masterKey.data(), passwordBytes.data(), copySize);
    }

    return masterKey;
}

QString MasterKeyManager::getContainerPath()
{
    return kVaultBasePath + QString("/password_container.bin");
}

