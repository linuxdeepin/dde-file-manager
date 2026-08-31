// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pathmanager.cpp
 * @brief Unit tests for PathManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/pathmanager.h"

#include <QTest>

using namespace dfmplugin_vault;

class PathManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PathManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PathManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PathManagerTest, createVaultMountDir)
{
    // Test method: bool createVaultMountDir((const QString &vaultBasePath))
    QString _arg0{};
    auto result = obj->createVaultMountDir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PathManagerTest, vaultLockPath)
{
    // Test getter: QString vaultLockPath()
    auto result = obj->vaultLockPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathManagerTest, vaultUnlockPath)
{
    // Test getter: QString vaultUnlockPath()
    auto result = obj->vaultUnlockPath();
    EXPECT_TRUE(result.isEmpty());

}
