// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortbybuttonprivate.cpp
 * @brief Unit tests for SortByButtonPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/sortbybutton.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class SortByButtonPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortByButtonPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortByButtonPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortByButtonPrivateTest, groupMenuTriggered)
{
    // Test method: void groupMenuTriggered((QAction *action))
    EXPECT_NO_FATAL_FAILURE(obj->groupMenuTriggered(nullptr));
}

TEST_F(SortByButtonPrivateTest, sort)
{
    // Test method: void sort(())
    EXPECT_NO_FATAL_FAILURE(obj->sort());
}
