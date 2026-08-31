// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultfilewatcherprivate.cpp
 * @brief Unit tests for VaultFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileutils/private/vaultfilewatcherprivate.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultFileWatcherPrivateTest, VaultFileWatcherPrivate)
{
    // Test constructor: VaultFileWatcherPrivate((const QUrl &fileUrl, VaultFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultFileWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(VaultFileWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}
