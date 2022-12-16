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
#ifndef VAULTCONFIGOPERATOR_H
#define VAULTCONFIGOPERATOR_H

#include "dfmplugin_utils_global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace dfmplugin_utils {
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

}
#endif // VAULTCONFIGOPERATOR_H
