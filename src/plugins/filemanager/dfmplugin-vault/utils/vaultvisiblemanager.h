// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTVISIBLEMANAGER_H
#define VAULTVISIBLEMANAGER_H
#include "dfmplugin_vault_global.h"

#include <QObject>

namespace dfmplugin_vault {
class VaultVisibleManager : public QObject
{
    Q_OBJECT
public:
    explicit VaultVisibleManager(QObject *parent = nullptr);

public:
    void infoRegister();
    void pluginServiceRegister();

public slots:
    void onWindowOpened(quint64 winID);
    void updateSideBarVaultItem();
    void removeSideBarVaultItem();
    void removeComputerVaultItem();
    void addVaultComputerMenu();

public:
    static VaultVisibleManager *instance();

private:
    //! true为已注册 false未注册
    bool infoRegisterState { false };
};
}
#endif   // VAULTVISIBLEMANAGER_H
