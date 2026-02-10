// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTERKEYMANAGER_H
#define MASTERKEYMANAGER_H

#include "dfmplugin_vault_global.h"

#include <QByteArray>
#include <QString>

namespace dfmplugin_vault {
class MasterKeyManager
{
public:
    static QByteArray generateMasterKey();

    static QByteArray generateMasterKeyFromPassword(const QString &password);

    static QString getContainerPath();
};
}

#endif   // MASTERKEYMANAGER_H

