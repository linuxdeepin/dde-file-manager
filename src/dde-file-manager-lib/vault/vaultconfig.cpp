// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultconfig.h"
#include "vaultglobaldefine.h"

#include <QDir>
#include <QSettings>

VaultConfig::VaultConfig(const QString &filePath)
    : m_filePath(filePath)
{
    if (filePath.isEmpty()) {
        m_filePath = VAULT_BASE_PATH + QDir::separator() + VAULT_CONFIG_FILE_NAME;
    }
    m_pSetting = new QSettings(m_filePath, QSettings::IniFormat);
}

VaultConfig::~VaultConfig()
{
    if (m_pSetting)
        delete m_pSetting;
    m_pSetting = nullptr;
}

void VaultConfig::set(const QString &nodeName, const QString &keyName, QVariant value)
{
    m_pSetting->setValue(QString("/%1/%2").arg(nodeName).arg(keyName), value);
}

QVariant VaultConfig::get(const QString &nodeName, const QString &keyName)
{
    return m_pSetting->value(QString("/%1/%2").arg(nodeName).arg(keyName));
}

QVariant VaultConfig::get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue)
{
    return m_pSetting->value(QString("/%1/%2").arg(nodeName).arg(keyName), defaultValue);
}
