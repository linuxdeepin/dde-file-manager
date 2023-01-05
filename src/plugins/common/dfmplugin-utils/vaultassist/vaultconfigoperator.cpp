/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "vaultconfigoperator.h"
#include "vaulthelper_global.h"

#include <dfmio_utils.h>

#include <QSettings>

DPUTILS_USE_NAMESPACE

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
