// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCONFIG_H
#define VAULTCONFIG_H

#include "dfmplugin_vault_global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE class QSettings;
QT_END_NAMESPACE

namespace dfmplugin_vault {

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
