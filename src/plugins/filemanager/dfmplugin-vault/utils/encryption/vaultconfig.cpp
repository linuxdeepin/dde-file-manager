// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultconfig.h"
#include "utils/vaultdefine.h"

#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QSettings>

using namespace dfmplugin_vault;

VaultConfig::VaultConfig(const QString &filePath)
    : currentFilePath(filePath)
{
    if (filePath.isEmpty()) {
        currentFilePath = DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(),
                                                         kVaultConfigFileName, nullptr);
    }
    pSetting = new QSettings(currentFilePath, QSettings::IniFormat);
}

VaultConfig::~VaultConfig()
{
    if (pSetting)
        delete pSetting;
    pSetting = nullptr;
}

void VaultConfig::set(const QString &nodeName, const QString &keyName, QVariant value)
{
    pSetting->setValue(QString("/%1/%2").arg(nodeName).arg(keyName), value);
}

QVariant VaultConfig::get(const QString &nodeName, const QString &keyName)
{
    QVariant var = pSetting->value(QString("/%1/%2").arg(nodeName).arg(keyName));
    return var;
}

QVariant VaultConfig::get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue)
{
    return pSetting->value(QString("/%1/%2").arg(nodeName).arg(keyName), defaultValue);
}

void VaultConfig::setVaultCreationType(const QString &type)
{
    set(kConfigNodeName, kConfigKeyVaultCreationType, type);
}

QString VaultConfig::getVaultCreationType() const
{
    return pSetting->value(QString("/%1/%2").arg(kConfigNodeName).arg(kConfigKeyVaultCreationType), "").toString();
}

bool VaultConfig::isNewCreated() const
{
    return getVaultCreationType() == kConfigValueVaultCreationTypeNew;
}
