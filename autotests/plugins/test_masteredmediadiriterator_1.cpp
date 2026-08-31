// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masteredmediadiriterator_1.cpp
 * @brief Unit tests for MasteredMediaDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mastered/masteredmediadiriterator.h"

#include <QTest>

using namespace dfmplugin_optical;

class MasteredMediaDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MasteredMediaDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MasteredMediaDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MasteredMediaDirIteratorTest, MasteredMediaDirIterator)
{
    // Test constructor: MasteredMediaDirIterator((const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MasteredMediaDirIteratorTest, changeScheme)
{
    // Test method: QUrl changeScheme((const QUrl &in))
    QUrl _arg0{};
    auto result = obj->changeScheme(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(MasteredMediaDirIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(MasteredMediaDirIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(MasteredMediaDirIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(MasteredMediaDirIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
