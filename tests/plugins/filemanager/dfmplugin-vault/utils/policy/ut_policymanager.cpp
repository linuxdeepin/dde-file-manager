// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/policy/policymanager.h"
#include "utils/vaultvisiblemanager.h"
#include "utils/vaulthelper.h"
#include "dbus/vaultdbusutils.h"

#include <gtest/gtest.h>

#include <QDBusConnection>

DPVAULT_USE_NAMESPACE

TEST(UT_PolicyManager, getVaultPolicy)
{
    stub_ext::StubExt stub;
    typedef bool(QDBusConnection::*FuncType)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<FuncType>(&QDBusConnection::connect), []{
        return true;
    });
    stub.set_lamda(&VaultDBusUtils::getVaultPolicy, []{
        return VaultPolicyState::kNotEnable;
    });

    VaultPolicyState state = PolicyManager::instance()->getVaultPolicy();

    EXPECT_TRUE(state == VaultPolicyState::kNotEnable);
}

TEST(UT_PolicyManager, setVaultPolicyState)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultDBusUtils::setVaultPolicyState, []{
        return true;
    });

    bool isOk = PolicyManager::instance()->setVaultPolicyState(VaultPolicyState::kNotEnable);

    EXPECT_TRUE(isOk);
}

TEST(UT_PolicyManager, getVaultCurrentPageMark)
{
    PolicyManager::instance()->recordVaultPageMark = PolicyManager::VaultPageMark::kUnknown;
    PolicyManager::VaultPageMark state = PolicyManager::instance()->getVaultCurrentPageMark();

    EXPECT_TRUE(state == PolicyManager::VaultPageMark::kUnknown);
}

TEST(UT_PolicyManager, setVauleCurrentPageMark)
{
    PolicyManager::instance()->setVauleCurrentPageMark(PolicyManager::VaultPageMark::kUnknown);

    EXPECT_TRUE(PolicyManager::instance()->recordVaultPageMark == PolicyManager::VaultPageMark::kUnknown);
}

TEST(UT_PolicyManager, isVaultVisiable)
{
    PolicyManager::instance()->vaultVisiable = false;
    EXPECT_FALSE(PolicyManager::instance()->isVaultVisiable());
}

TEST(UT_PolicyManager, slotVaultPolicy_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kNotEnable;
    });
    stub.set_lamda(&PolicyManager::getVaultCurrentPageMark, []{
        return PolicyManager::VaultPageMark::kUnknown;
    });

    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_FALSE(PolicyManager::instance()->vaultVisiable);
}

TEST(UT_PolicyManager, slotVaultPolicy_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kNotEnable;
    });
    stub.set_lamda(&PolicyManager::getVaultCurrentPageMark, []{
        return PolicyManager::VaultPageMark::kClipboardPage;
    });
    stub.set_lamda(&VaultVisibleManager::removeSideBarVaultItem, []{});
    stub.set_lamda(&VaultVisibleManager::removeComputerVaultItem, []{});

    PolicyManager::instance()->vaultVisiable = true;
    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_FALSE(PolicyManager::instance()->vaultVisiable);
}

TEST(UT_PolicyManager, slotVaultPolicy_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kNotEnable;
    });
    stub.set_lamda(&PolicyManager::getVaultCurrentPageMark, []{
        return PolicyManager::VaultPageMark::kCopyFilePage;
    });
    stub.set_lamda(&VaultHelper::lockVault, []{});
    stub.set_lamda(&VaultVisibleManager::removeSideBarVaultItem, []{});
    stub.set_lamda(&VaultVisibleManager::removeComputerVaultItem, []{});

    PolicyManager::instance()->vaultVisiable = true;
    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_FALSE(PolicyManager::instance()->vaultVisiable);
}

TEST(UT_PolicyManager, slotVaultPolicy_four)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kNotEnable;
    });
    stub.set_lamda(&PolicyManager::getVaultCurrentPageMark, []{
        return PolicyManager::VaultPageMark::kCreateVaultPage1;
    });
    stub.set_lamda(&PolicyManager::setVaultPolicyState, [ &isOk ]{
        isOk = true;
        return true;
    });

    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_TRUE(isOk);
}

TEST(UT_PolicyManager, slotVaultPolicy_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kEnable;
    });
    stub.set_lamda(&VaultVisibleManager::infoRegister, []{});
    stub.set_lamda(&VaultVisibleManager::pluginServiceRegister, []{});
    stub.set_lamda(&VaultVisibleManager::addSideBarVaultItem, []{});
    stub.set_lamda(&VaultVisibleManager::addComputer, []{});

    PolicyManager::instance()->vaultVisiable = false;
    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_TRUE(PolicyManager::instance()->vaultVisiable);
}

TEST(UT_PolicyManager, slotVaultPolicy_six)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&PolicyManager::getVaultPolicy, []{
        return VaultPolicyState::kUnkonw;
    });

    PolicyManager::instance()->vaultVisiable = false;
    PolicyManager::instance()->slotVaultPolicy();

    EXPECT_FALSE(PolicyManager::instance()->vaultVisiable);
}
