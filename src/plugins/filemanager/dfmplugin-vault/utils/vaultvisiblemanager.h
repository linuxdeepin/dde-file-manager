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
#ifndef VAULTVISIBLEMANAGER_H
#define VAULTVISIBLEMANAGER_H
#include "dfmplugin_vault_global.h"

#include <QObject>

DPVAULT_BEGIN_NAMESPACE
class VaultVisibleManager : public QObject
{
    Q_OBJECT
public:
    explicit VaultVisibleManager(QObject *parent = nullptr);

public:
    /*!
    * \brief isVaultEnabled
    * \return true vault is available, vice versa.
    */
    bool isVaultEnabled();

    void infoRegister();

    void pluginServiceRegister();

public slots:
    void onWindowOpened(quint64 winID);

    void addSideBarVaultItem();

    void addComputer();

    void addFileOperations();

    void removeSideBarVaultItem();

    void removeComputerVaultItem();

public:
    static VaultVisibleManager *instance();

private:
    //! true为已注册 false未注册
    bool infoRegisterState { false };
    bool serviceRegisterState { false };
};
DPVAULT_END_NAMESPACE
#endif   // VAULTVISIBLEMANAGER_H
