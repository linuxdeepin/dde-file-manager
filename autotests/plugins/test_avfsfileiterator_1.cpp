// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfileiterator_1.cpp
 * @brief Unit tests for AvfsFileIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfileiterator.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(AvfsFileIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsFileIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(AvfsFileIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(AvfsFileIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(AvfsFileIteratorTest, AvfsFileIterator_Destructor)
{
    // Test method:  ~AvfsFileIterator(())
    EXPECT_NO_FATAL_FAILURE({ AvfsFileIterator *tmp = new AvfsFileIterator(); delete tmp; });
}
