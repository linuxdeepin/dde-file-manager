// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashdiriterator.cpp
 * @brief Unit tests for TrashDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trashdiriterator.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashDirIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(TrashDirIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(TrashDirIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
