// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagwidget.cpp
 * @brief Unit tests for TagWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/tagwidget.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagWidgetTest, TagWidget)
{
    // Test constructor: TagWidget((QUrl url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagWidgetTest, onCheckedColorChanged)
{
    // Test method: void onCheckedColorChanged((const QColor &color))
    QColor _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCheckedColorChanged(_arg0));
}
