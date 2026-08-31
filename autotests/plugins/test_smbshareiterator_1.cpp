// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbshareiterator_1.cpp
 * @brief Unit tests for SmbShareIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "iterator/smbshareiterator.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class SmbShareIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbShareIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbShareIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbShareIteratorTest, SmbShareIterator)
{
    // Test constructor: SmbShareIterator((const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbShareIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SmbShareIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(SmbShareIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(SmbShareIteratorTest, SmbShareIterator_Destructor)
{
    // Test method:  ~SmbShareIterator(())
    EXPECT_NO_FATAL_FAILURE({ SmbShareIterator *tmp = new SmbShareIterator(); delete tmp; });
}
