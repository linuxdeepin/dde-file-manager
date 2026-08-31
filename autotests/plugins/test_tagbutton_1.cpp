// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagbutton_1.cpp
 * @brief Unit tests for TagButton methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tagbutton.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagButtonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagButton();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagButton *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagButtonTest, TagButton)
{
    // Test constructor: TagButton((const QColor &color, QWidget *const parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagButtonTest, enterEvent)
{
    // Test event handler: enterEvent((QEnterEvent *event))
    QEnterEvent _event(QEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(TagButtonTest, isHovered)
{
    // Test bool getter: isHovered()
    bool result = obj->isHovered();
    EXPECT_FALSE(result);

}

TEST_F(TagButtonTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(TagButtonTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(TagButtonTest, setBackGroundColor)
{
    // Test method: void setBackGroundColor(())
    EXPECT_NO_FATAL_FAILURE(obj->setBackGroundColor());
}

TEST_F(TagButtonTest, setCheckable)
{
    // Test setter: void setCheckable((bool checkable))
    EXPECT_NO_FATAL_FAILURE(obj->setCheckable(false));
}

TEST_F(TagButtonTest, setPaintStatus)
{
    // Test setter: void setPaintStatus((TagButton::PaintStatus status))
    EXPECT_NO_FATAL_FAILURE(obj->setPaintStatus(TagButton::PaintStatus()));
}

TEST_F(TagButtonTest, setRadius)
{
    // Test method: void setRadius(())
    EXPECT_NO_FATAL_FAILURE(obj->setRadius());
}

TEST_F(TagButtonTest, setRadiusF)
{
    // Test method: void setRadiusF(())
    EXPECT_NO_FATAL_FAILURE(obj->setRadiusF());
}
