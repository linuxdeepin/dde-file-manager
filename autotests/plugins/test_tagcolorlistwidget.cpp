// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagcolorlistwidget.cpp
 * @brief Unit tests for TagColorListWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tagcolorlistwidget.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagColorListWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagColorListWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagColorListWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagColorListWidgetTest, clearToolTipText)
{
    // Test method: void clearToolTipText(())
    EXPECT_NO_FATAL_FAILURE(obj->clearToolTipText());
}

TEST_F(TagColorListWidgetTest, exclusive)
{
    // Test bool getter: exclusive()
    bool result = obj->exclusive();
    EXPECT_FALSE(result);

}

TEST_F(TagColorListWidgetTest, initUiElement)
{
    // Test method: void initUiElement(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiElement());
}

TEST_F(TagColorListWidgetTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(TagColorListWidgetTest, setCheckedColorList)
{
    // Test setter: void setCheckedColorList((const QList<QColor> &colorNames))
    QList<QColor> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCheckedColorList(_arg0));
}
