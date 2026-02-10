// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCONFIGOPERATOR_H
#define VAULTCONFIGOPERATOR_H

#include "daemonplugin_vaultdaemon_global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

DAEMONPVAULT_BEGIN_NAMESPACE

inline constexpr char kConfigNodeName[] { "INFO" };
inline constexpr char kConfigKeyEncryptionMethod[] { "encryption_method" };
inline constexpr char kConfigValueMethodKey[] { "key_encryption" };
inline constexpr char kConfigValueMethodTransparent[] { "transparent_encryption" };
inline constexpr char kConfigKeyNotExist[] { "NoExist" };
inline constexpr char kConfigKeyAlgoName[] { "algoName" };

class VaultConfigOperator
{
public:
    VaultConfigOperator(const QString &filePath = "");
    ~VaultConfigOperator();
    void set(const QString &nodeName, const QString &keyName, QVariant value);
    QVariant get(const QString &nodeName, const QString &keyName);
    QVariant get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue);

private:
    QString currentFilePath;
    QSettings *setting;
};
DAEMONPVAULT_END_NAMESPACE

#endif // VAULTCONFIGOPERATOR_H
