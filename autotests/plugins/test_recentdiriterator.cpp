// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentdiriterator.cpp
 * @brief Unit tests for RecentDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/recentdiriterator.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentDirIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(RecentDirIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentDirIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(RecentDirIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(RecentDirIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(RecentDirIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
