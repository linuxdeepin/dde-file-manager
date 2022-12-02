// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
