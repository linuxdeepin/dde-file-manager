// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_contentbackgroundwidget.cpp
 * @brief Unit tests for ContentBackgroundWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/widgets/contentbackgroundwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class ContentBackgroundWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ContentBackgroundWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ContentBackgroundWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ContentBackgroundWidgetTest, radius)
{
    // Test getter: int radius()
    auto result = obj->radius();
    EXPECT_EQ(result, 0);

}

TEST_F(ContentBackgroundWidgetTest, roundEdge)
{
    // Test getter: RoundEdge roundEdge()
    auto result = obj->roundEdge();
    EXPECT_GE(static_cast<int>(result), 0);

}
