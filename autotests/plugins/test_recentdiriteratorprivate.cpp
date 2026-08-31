// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentdiriteratorprivate.cpp
 * @brief Unit tests for RecentDirIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/recentdiriterator.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentDirIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentDirIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentDirIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentDirIteratorPrivateTest, RecentDirIteratorPrivate_Destructor)
{
    // Test method:  ~RecentDirIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ RecentDirIteratorPrivate *tmp = new RecentDirIteratorPrivate(); delete tmp; });
}
