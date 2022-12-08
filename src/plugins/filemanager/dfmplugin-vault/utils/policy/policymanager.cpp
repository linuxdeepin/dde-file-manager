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
#include "policymanager.h"
#include "dbus/vaultdbusutils.h"
#include "utils/vaulthelper.h"
#include "utils/vaultvisiblemanager.h"

#include <QDBusConnection>

using namespace dfmplugin_vault;
PolicyManager::VaultPageMark PolicyManager::recordVaultPageMark;
bool PolicyManager::vaultVisiable = true;
PolicyManager::PolicyManager(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().connect("com.deepin.filemanager.daemon",
                                         "/com/deepin/filemanager/daemon/AccessControlManager",
                                         "com.deepin.filemanager.daemon.AccessControlManager",
                                         "AccessVaultPolicyNotify",
                                         this,
                                         SLOT(slotVaultPolicy()));
}

int PolicyManager::getVaultPolicy()
{
    return VaultDBusUtils::getVaultPolicy();
}

bool PolicyManager::setVaultPolicyState(int policyState)
{
    return VaultDBusUtils::setVaultPolicyState(policyState);
}

PolicyManager::VaultPageMark PolicyManager::getVaultCurrentPageMark()
{
    return recordVaultPageMark;
}

void PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark mark)
{
    recordVaultPageMark = mark;
}

bool PolicyManager::isVaultVisiable()
{
    return vaultVisiable;
}

PolicyManager *PolicyManager::instance()
{
    static PolicyManager obj;
    return &obj;
}

void PolicyManager::slotVaultPolicy()
{
    switch (getVaultPolicy()) {
    case 1: {
        switch (getVaultCurrentPageMark()) {
        case VaultPageMark::kUnknown:
            vaultVisiable = false;
            break;
        case VaultPageMark::kClipboardPage:
            if (vaultVisiable) {
                VaultHelper::instance()->lockVault(true);
                vaultVisiable = false;
                VaultVisibleManager::instance()->removeSideBarVaultItem();
                VaultVisibleManager::instance()->removeComputerVaultItem();
                VaultHelper::instance()->killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::kCopyFilePage:
            if (vaultVisiable) {
                VaultHelper::instance()->lockVault(true);
                vaultVisiable = false;
                VaultVisibleManager::instance()->removeSideBarVaultItem();
                VaultVisibleManager::instance()->removeComputerVaultItem();
                VaultHelper::instance()->killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::kCreateVaultPage1:
        case VaultPageMark::kUnlockVaultPage:
        case VaultPageMark::kDeleteFilePage:
        case VaultPageMark::kDeleteVaultPage:
        case VaultPageMark::kVaultPage:
        case VaultPageMark::kRetrievePassWordPage:
        case VaultPageMark::kCreateVaultPage:
            setVaultPolicyState(2);
            return;
        }

        VaultVisibleManager::instance()->removeSideBarVaultItem();
        VaultVisibleManager::instance()->removeComputerVaultItem();
    } break;
    case 2:
        if (!vaultVisiable) {
            vaultVisiable = true;
            VaultVisibleManager::instance()->infoRegister();
            VaultVisibleManager::instance()->pluginServiceRegister();
            VaultVisibleManager::instance()->addSideBarVaultItem();
            VaultVisibleManager::instance()->addComputer();
        }
        break;
    }
}
