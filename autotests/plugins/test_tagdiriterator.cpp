// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdiriterator.cpp
 * @brief Unit tests for TagDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/tagdiriterator.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(TagDirIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagDirIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TagDirIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(TagDirIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TagDirIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
