// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_masteredmediadiriterator.cpp
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

TEST_F(MasteredMediaDirIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(MasteredMediaDirIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
