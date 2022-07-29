/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
