// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEUTILS_H
#define UPGRADEUTILS_H

#include <QVariant>

namespace dfm_upgrade {
namespace UpgradeUtils {
QVariant genericAttribute(const QString &key);
void addOldGenericAttribute(const QJsonArray &array);
QVariant applicationAttribute(const QString &key);
bool backupFile(const QString &sourceFile, const QString &backupDirPath);
}
}

#endif   // UPGRADEUTILS_H
