// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCONFIG_H
#define VAULTCONFIG_H

#define CONFIG_NODE_NAME                        "INFO"
#define CONFIG_KEY_CIPHER                       "pbkgcipher"
#define CONFIG_KEY_VERSION                      "version"
#define CONFIG_VAULT_VERSION                    "new"
#define CONFIG_VAULT_VERSION_1050               "1050"
#define CONFIG_KEY_ALGONAME                     "algoName"
#define CONFIG_KEY_ENCRYPTION_METHOD            "encryption_method"
#define CONFIG_METHOD_VALUE_KEY                 "key_encryption"
#define CONFIG_METHOD_VALUE_TRANSPARENT         "transparent_encryption"
#define CONFIG_KEY_USE_USER_PASSWORD            "use_user_password"

#define GROUP_POLICY_VAULT_ALGO_NAME    "dfm.vault.algo.name"
#define DEFAULT_AES_ALGO_NAME           "aes-256-gcm"
#define DEFAULT_SM4_ALGO_NAME           "sm4-128-ecb"

#include <QVariant>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

class VaultConfig
{
public:
    explicit VaultConfig(const QString &filePath = "");
    ~VaultConfig();
    void set(const QString &nodeName, const QString &keyName, QVariant value);
    QVariant get(const QString &nodeName, const QString &keyName);
    QVariant get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue);

private:
    QString m_filePath;
    QSettings *m_pSetting;
};

#endif // VAULTCONFIG_H
