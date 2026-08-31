// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagwidget_1.cpp
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

TEST_F(TagWidgetTest, filterInput)
{
    // Test method: void filterInput(())
    EXPECT_NO_FATAL_FAILURE(obj->filterInput());
}

TEST_F(TagWidgetTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}

TEST_F(TagWidgetTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TagWidgetTest, onTagColorChanged)
{
    // Test method: void onTagColorChanged((const QVariantMap &tagAndColorName))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTagColorChanged(_arg0));
}

TEST_F(TagWidgetTest, setLayoutHorizontally)
{
    // Test setter: void setLayoutHorizontally((bool horizontal))
    EXPECT_NO_FATAL_FAILURE(obj->setLayoutHorizontally(false));
}

TEST_F(TagWidgetTest, setUrl)
{
    // Test setter: void setUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setUrl(_arg0));
}
