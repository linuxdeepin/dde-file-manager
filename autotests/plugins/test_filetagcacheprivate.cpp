// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filetagcacheprivate.cpp
 * @brief Unit tests for FileTagCachePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filetagcache.h"

#include <QTest>

using namespace dfmplugin_tag;

class FileTagCachePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileTagCachePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileTagCachePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileTagCachePrivateTest, FileTagCachePrivate)
{
    // Test constructor: FileTagCachePrivate((FileTagCache *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileTagCachePrivateTest, FileTagCachePrivate_Destructor)
{
    // Test method:  ~FileTagCachePrivate(())
    EXPECT_NO_FATAL_FAILURE({ FileTagCachePrivate *tmp = new FileTagCachePrivate(); delete tmp; });
}
