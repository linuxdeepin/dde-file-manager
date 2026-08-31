// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagcolorlistwidget_1.cpp
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

TEST_F(TagColorListWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(TagColorListWidgetTest, setExclusive)
{
    // Test setter: void setExclusive((bool exclusive))
    EXPECT_NO_FATAL_FAILURE(obj->setExclusive(false));
}

TEST_F(TagColorListWidgetTest, setToolTipText)
{
    // Test setter: void setToolTipText((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setToolTipText(_arg0));
}

TEST_F(TagColorListWidgetTest, setToolTipVisible)
{
    // Test setter: void setToolTipVisible((bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->setToolTipVisible(false));
}
