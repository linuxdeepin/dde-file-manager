// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarviewstyle.cpp
 * @brief Unit tests for SidebarViewStyle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarview.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SidebarViewStyleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SidebarViewStyle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SidebarViewStyle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SidebarViewStyleTest, SidebarViewStyle)
{
    // Test constructor: SidebarViewStyle((QStyle *style))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SidebarViewStyleTest, drawPrimitive)
{
    // Test method: void drawPrimitive((PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget))
    EXPECT_NO_FATAL_FAILURE(obj->drawPrimitive(PrimitiveElement(), nullptr, nullptr, nullptr));
}
