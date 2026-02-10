// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTUTILS_H
#define VAULTUTILS_H

#include "dfmplugin_vault_global.h"

#include <polkit-qt6-1/PolkitQt1/Authority>

namespace dfmplugin_vault {

class VaultUtils : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultUtils)

public:
    static VaultUtils &instance();

    void showAuthorityDialog(const QString &actionId);

Q_SIGNALS:
    void resultOfAuthority(bool ok);

private slots:
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

private:
    explicit VaultUtils(QObject *parent = Q_NULLPTR);
};

}

#endif   // vaultutils_H
