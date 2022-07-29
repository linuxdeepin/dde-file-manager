/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
