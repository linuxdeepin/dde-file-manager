// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagbutton.cpp
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

TEST_F(TagButtonTest, color)
{
    // Test getter: QColor color()
    auto result = obj->color();
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagButtonTest, isChecked)
{
    // Test bool getter: isChecked()
    bool result = obj->isChecked();
    EXPECT_FALSE(result);

}

TEST_F(TagButtonTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(TagButtonTest, setChecked)
{
    // Test setter: void setChecked((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->setChecked(false));
}
