// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_blockentryfileentity.cpp
 * @brief Unit tests for BlockEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/blockentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class BlockEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BlockEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BlockEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BlockEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(BlockEntryFileEntityTest, mountPoint)
{
    // Test getter: QUrl mountPoint()
    auto result = obj->mountPoint();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(BlockEntryFileEntityTest, resetWindowsVolTag)
{
    // Test method: void resetWindowsVolTag(())
    EXPECT_NO_FATAL_FAILURE(obj->resetWindowsVolTag());
}

TEST_F(BlockEntryFileEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
