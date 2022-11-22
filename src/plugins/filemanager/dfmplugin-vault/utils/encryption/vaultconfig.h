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

namespace dfmplugin_vault {
inline constexpr char kConfigNodeName[] { "INFO" };
inline constexpr char kConfigKeyCipher[] { "pbkgcipher" };
inline constexpr char kConfigKeyVersion[] { "version" };
inline constexpr char kConfigKeyUseUserPassWord[] { "use_user_password" };
inline constexpr char kConfigVaultVersion[] { "new" };
inline constexpr char kConfigVaultVersion1050[] { "1050" };

inline constexpr char kConfigKeyNotExist[] { "NoExist" };

class VaultConfig
{
public:
    explicit VaultConfig(const QString &filePath = "");
    ~VaultConfig();
    void set(const QString &nodeName, const QString &keyName, QVariant value);
    QVariant get(const QString &nodeName, const QString &keyName);
    QVariant get(const QString &nodeName, const QString &keyName, const QVariant &defaultValue);

private:
    QString currentFilePath;
    QSettings *pSetting;
};
}
#endif   // VAULTCONFIG_H
