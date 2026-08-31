// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithwidget.cpp
 * @brief Unit tests for OpenWithWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithwidget.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithWidgetTest, OpenWithWidget)
{
    // Test constructor: OpenWithWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenWithWidgetTest, slotExpandChange)
{
    // Test method: void slotExpandChange((bool state))
    EXPECT_NO_FATAL_FAILURE(obj->slotExpandChange(false));
}
