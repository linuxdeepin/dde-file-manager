// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vault.cpp
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

TEST_F(VaultTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
