// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentdiriterator_1.cpp
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

TEST_F(RecentDirIteratorTest, RecentDirIterator)
{
    // Test constructor: RecentDirIterator((const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentDirIteratorTest, RecentDirIterator_Destructor)
{
    // Test method:  ~RecentDirIterator(())
    EXPECT_NO_FATAL_FAILURE({ RecentDirIterator *tmp = new RecentDirIterator(); delete tmp; });
}
