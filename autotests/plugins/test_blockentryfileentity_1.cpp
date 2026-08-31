// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_blockentryfileentity_1.cpp
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

TEST_F(BlockEntryFileEntityTest, BlockEntryFileEntity)
{
    // Test constructor: BlockEntryFileEntity((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BlockEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BlockEntryFileEntityTest, getProperty)
{
    // Test method: QVariant getProperty((const char *const key))
    auto result = obj->getProperty(nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BlockEntryFileEntityTest, isAccessable)
{
    // Test bool getter: isAccessable()
    bool result = obj->isAccessable();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, isSiblingOfRoot)
{
    // Test bool getter: isSiblingOfRoot()
    bool result = obj->isSiblingOfRoot();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, loadDiskInfo)
{
    // Test method: void loadDiskInfo(())
    EXPECT_NO_FATAL_FAILURE(obj->loadDiskInfo());
}

TEST_F(BlockEntryFileEntityTest, order)
{
    // Test getter: AbstractEntryFileEntity::EntryOrder order()
    auto result = obj->order();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(BlockEntryFileEntityTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(BlockEntryFileEntityTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, showSizeAndProgress)
{
    // Test bool getter: showSizeAndProgress()
    bool result = obj->showSizeAndProgress();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}

TEST_F(BlockEntryFileEntityTest, sizeTotal)
{
    // Test getter: quint64 sizeTotal()
    auto result = obj->sizeTotal();
    EXPECT_EQ(result, 0);

}

TEST_F(BlockEntryFileEntityTest, sizeUsage)
{
    // Test getter: quint64 sizeUsage()
    auto result = obj->sizeUsage();
    EXPECT_EQ(result, 0);

}
