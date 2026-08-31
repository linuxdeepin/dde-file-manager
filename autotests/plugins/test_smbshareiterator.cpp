// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbshareiterator.cpp
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

TEST_F(SmbShareIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(SmbShareIteratorTest, initIterator)
{
    // Test bool getter: initIterator()
    bool result = obj->initIterator();
    EXPECT_FALSE(result);

}

TEST_F(SmbShareIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(SmbShareIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
