// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareiterator_1.cpp
 * @brief Unit tests for ShareIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "iterator/shareiterator.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(ShareIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ShareIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ShareIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(ShareIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ShareIteratorTest, ShareIterator_Destructor)
{
    // Test method:  ~ShareIterator(())
    EXPECT_NO_FATAL_FAILURE({ ShareIterator *tmp = new ShareIterator(); delete tmp; });
}
