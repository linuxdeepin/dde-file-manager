// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stub.h"

#define private public
#include "vault/vaultdbusresponse.h"

namespace {
class TestVaultDbusResponse : public testing::Test
{
public:
    VaultDbusResponse *ins { NULL };
    void SetUp() override
    {
        std::cout << "start TestVaultDbusResponse";
        ins = VaultDbusResponse::instance();
    }
    void TearDown() override
    {
        std::cout << "end TestVaultDbusResponse";
    }
};

TEST_F(TestVaultDbusResponse, transparentUnlockVault)
{
    VaultDbusResponse::VaultState(*stub_state)() = []()->VaultDbusResponse::VaultState{
        return VaultDbusResponse::NotAvailable;
    };
    Stub stu;
    stu.set(&VaultDbusResponse::state, stub_state);
    EXPECT_NO_FATAL_FAILURE(ins->transparentUnlockVault());
}

TEST_F(TestVaultDbusResponse, ChangeJson)
{
    EXPECT_NO_FATAL_FAILURE(ins->ChangeJson("~/test/1.txt", "test", "test", "test"));
}

}
