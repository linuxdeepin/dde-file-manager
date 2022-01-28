/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "dfmplugin_vault_global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE class QSettings;
QT_END_NAMESPACE

DPVAULT_BEGIN_NAMESPACE
constexpr char kConfigNodeName[] { "INFO" };
constexpr char kConfigKeyCipher[] { "pbkgcipher" };
constexpr char kConfigKeyVersion[] { "version" };
constexpr char kConfigVaultVersion[] { "new" };
constexpr char kConfigVaultVersion1050[] { "1050" };
class VaultConfig
{
public:
    explicit VaultConfig(const QString &filePath = "");
    ~VaultConfig();
    void set(const QString &nodeName, const QString &keyName, QVariant value);
    QVariant get(const QString &nodeName, const QString &keyName);

private:
    QString currentFilePath;
    QSettings *pSetting;
};
DPVAULT_END_NAMESPACE
#endif   // VAULTCONFIG_H
