// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultentryfileentity.cpp
 * @brief Unit tests for VaultEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultentryfileentity.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultEntryFileEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}
