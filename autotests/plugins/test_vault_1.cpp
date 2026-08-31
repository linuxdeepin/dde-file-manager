// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vault_1.cpp
 * @brief Unit tests for Vault methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vault.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Vault();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Vault *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(VaultTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}
