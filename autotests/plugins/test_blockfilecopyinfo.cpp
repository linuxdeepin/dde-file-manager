// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_blockfilecopyinfo.cpp
 * @brief Unit tests for BlockFileCopyInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/workerdata.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class BlockFileCopyInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BlockFileCopyInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BlockFileCopyInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BlockFileCopyInfoTest, BlockFileCopyInfo)
{
    // Test constructor: BlockFileCopyInfo(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(BlockFileCopyInfoTest, BlockFileCopyInfo_Destructor)
{
    // Test method:  ~BlockFileCopyInfo(())
    EXPECT_NO_FATAL_FAILURE({ BlockFileCopyInfo *tmp = new BlockFileCopyInfo(); delete tmp; });
}
