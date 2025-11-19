// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCONFIG_H
#define VAULTCONFIG_H

#include "dfmplugin_vault_global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE class QSettings;
QT_END_NAMESPACE

namespace dfmplugin_vault {
inline constexpr char kConfigNodeName[] { "INFO" };
inline constexpr char kConfigKeyCipher[] { "pbkgcipher" };
inline constexpr char kConfigKeyVersion[] { "version" };
inline constexpr char kConfigKeyUseUserPassWord[] { "use_user_password" };
inline constexpr char kConfigVaultVersion[] { "new" };
inline constexpr char kConfigVaultVersion1050[] { "1050" };
inline constexpr char kConfigKeyAlgoName[] { "algoName" };
inline constexpr char kConfigKeyEncryptionMethod[] { "encryption_method" };
inline constexpr char kConfigValueMethodKey[] { "key_encryption" };
inline constexpr char kConfigValueMethodTransparent[] { "transparent_encryption" };
inline constexpr char kConfigKeyNotExist[] { "NoExist" };
inline constexpr char kGroupPolicyKeyVaultAlgoName[] { "dfm.vault.algo.name" };
inline constexpr char kConfigKeyVaultCreationType[] { "creation_type" };
inline constexpr char kConfigValueVaultCreationTypeNew[] { "new_created" };
inline constexpr char kConfigValueVaultCreationTypeMigrated[] { "migrated" };

class VaultConfig
{
public:
    explicit VaultConfig(const QString &filePath = "");
    ~VaultConfig();
    void set(const QString &nodeName, const QString &keyName, QVariant value);
    QVariant get(const QString &nodeName, const QString &keyName);
    QVariant get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue);

    void setVaultCreationType(const QString &type);
    QString getVaultCreationType() const;
    bool isNewCreated() const;

private:
    QString currentFilePath;
    QSettings *pSetting;
};
}
#endif   // VAULTCONFIG_H
