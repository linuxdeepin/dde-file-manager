// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

VaultPolicyState PolicyManager::getVaultPolicy()
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
    case VaultPolicyState::kNotEnable: {
        switch (getVaultCurrentPageMark()) {
        case VaultPageMark::kUnknown:
            vaultVisiable = false;
            break;
        case VaultPageMark::kClipboardPage:
            if (vaultVisiable) {
                if (!VaultHelper::instance()->lockVault(false)) {
                    fmWarning() << "Lock vault failed!";
                    return;
                }
                vaultVisiable = false;
                VaultVisibleManager::instance()->removeSideBarVaultItem();
                VaultVisibleManager::instance()->removeComputerVaultItem();
                VaultHelper::instance()->killVaultTasks();
                return;
            }
            break;
        case VaultPageMark::kCopyFilePage:
            if (vaultVisiable) {
                if (!VaultHelper::instance()->lockVault(false)) {
                    fmWarning() << "Lock vault failed!";
                    return;
                }
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
    case VaultPolicyState::kEnable:
        if (!vaultVisiable) {
            vaultVisiable = true;
            VaultVisibleManager::instance()->infoRegister();
            VaultVisibleManager::instance()->pluginServiceRegister();
            VaultVisibleManager::instance()->updateSideBarVaultItem();
        }
        break;
    default:
        break;
    }
}
