// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultconfigoperator.h"

#include <dfm-io/dfmio_utils.h>

#include <QSettings>

DAEMONPVAULT_USE_NAMESPACE

VaultConfigOperator::VaultConfigOperator(const QString &filePath)
    : currentFilePath(filePath)
{
    if (filePath.isEmpty()) {
        currentFilePath = DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(),
                                                         kVaultConfigFileName, nullptr);
    }
    setting = new QSettings(currentFilePath, QSettings::IniFormat);
    
    fmDebug() << "[VaultConfigOperator::VaultConfigOperator] Initialized with config file:" << currentFilePath;
}

VaultConfigOperator::~VaultConfigOperator()
{
    if (setting) {
        fmDebug() << "[VaultConfigOperator::~VaultConfigOperator] Destroying config operator for:" << currentFilePath;
        delete setting;
    }
    setting = nullptr;
}

void VaultConfigOperator::set(const QString &nodeName, const QString &keyName, QVariant value)
{
    QString fullKey = QString("/%1/%2").arg(nodeName).arg(keyName);
    setting->setValue(fullKey, value);
    fmDebug() << "[VaultConfigOperator::set] Set config value - key:" << fullKey << "value:" << value.toString();
}

QVariant VaultConfigOperator::get(const QString &nodeName, const QString &keyName)
{
    QString fullKey = QString("/%1/%2").arg(nodeName).arg(keyName);
    QVariant value = setting->value(fullKey);
    fmDebug() << "[VaultConfigOperator::get] Retrieved config value - key:" << fullKey << "value:" << value.toString();
    return value;
}

QVariant VaultConfigOperator::get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue)
{
    QString fullKey = QString("/%1/%2").arg(nodeName).arg(keyName);
    QVariant value = setting->value(fullKey, defaultValue);
    fmDebug() << "[VaultConfigOperator::get] Retrieved config value with default - key:" << fullKey << "value:" << value.toString() << "default:" << defaultValue.toString();
    return value;
}
