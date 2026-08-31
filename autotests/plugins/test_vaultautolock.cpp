// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultautolock.cpp
 * @brief Unit tests for VaultAutoLock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultautolock.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultAutoLockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultAutoLock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultAutoLock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultAutoLockTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(VaultAutoLockTest, resetConfig)
{
    // Test method: void resetConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->resetConfig());
}
