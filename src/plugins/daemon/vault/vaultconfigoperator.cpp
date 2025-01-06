// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
}

VaultConfigOperator::~VaultConfigOperator()
{
    if (setting)
        delete setting;
    setting = nullptr;
}

void VaultConfigOperator::set(const QString &nodeName, const QString &keyName, QVariant value)
{
    setting->setValue(QString("/%1/%2").arg(nodeName).arg(keyName), value);
}

QVariant VaultConfigOperator::get(const QString &nodeName, const QString &keyName)
{
    return setting->value(QString("/%1/%2").arg(nodeName).arg(keyName));
}

QVariant VaultConfigOperator::get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue)
{
    return setting->value(QString("/%1/%2").arg(nodeName).arg(keyName), defaultValue);
}
