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
    QVariant var = m_pSetting->value(QString("/%1/%2").arg(nodeName).arg(keyName));
    return var;
}
