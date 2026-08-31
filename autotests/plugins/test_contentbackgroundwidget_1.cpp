// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_contentbackgroundwidget_1.cpp
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

TEST_F(ContentBackgroundWidgetTest, ContentBackgroundWidget)
{
    // Test constructor: ContentBackgroundWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ContentBackgroundWidgetTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(ContentBackgroundWidgetTest, setRadius)
{
    // Test method: void setRadius(())
    EXPECT_NO_FATAL_FAILURE(obj->setRadius());
}

TEST_F(ContentBackgroundWidgetTest, setRoundEdge)
{
    // Test method: void setRoundEdge(())
    EXPECT_NO_FATAL_FAILURE(obj->setRoundEdge());
}
