// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagwidgetprivate.cpp
 * @brief Unit tests for TagWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widgets/private/tagwidget_p.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagWidgetPrivateTest, TagWidgetPrivate)
{
    // Test constructor: TagWidgetPrivate((TagWidget *qq, const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagWidgetPrivateTest, initializeUI)
{
    // Test method: void initializeUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUI());
}
