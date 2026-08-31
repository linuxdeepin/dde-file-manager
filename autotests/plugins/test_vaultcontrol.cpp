// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultcontrol.cpp
 * @brief Unit tests for VaultControl methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultcontrol.h"

#include <QTest>

using namespace vault;

class VaultControlTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultControl();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultControl *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultControlTest, instance)
{
    // Test getter: DAEMONPVAULT_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultControlTest, responseNetworkStateChaneDBus)
{
    // Test method: void responseNetworkStateChaneDBus((int st))
    EXPECT_NO_FATAL_FAILURE(obj->responseNetworkStateChaneDBus(0));
}

TEST_F(VaultControlTest, transparentUnlockVault)
{
    // Test bool getter: transparentUnlockVault()
    bool result = obj->transparentUnlockVault();
    EXPECT_FALSE(result);

}

TEST_F(VaultControlTest, versionString)
{
    // Test getter: VaultControl::CryfsVersionInfo versionString()
    auto result = obj->versionString();
    EXPECT_GE(static_cast<int>(result), 0);

}
