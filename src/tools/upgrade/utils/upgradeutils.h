// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPGRADEUTILS_H
#define UPGRADEUTILS_H

#include <QVariant>

namespace dfm_upgrade {
namespace UpgradeUtils {
QVariant genericAttribute(const QString &key);
QVariant applicationAttribute(const QString &key);
}
}

#endif   // UPGRADEUTILS_H
