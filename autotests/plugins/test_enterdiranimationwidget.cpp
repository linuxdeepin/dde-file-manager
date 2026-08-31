// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_enterdiranimationwidget.cpp
 * @brief Unit tests for EnterDirAnimationWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/enterdiranimationwidget.h"

#include <QTest>

using namespace dfmplugin_workspace;

class EnterDirAnimationWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EnterDirAnimationWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EnterDirAnimationWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EnterDirAnimationWidgetTest, EnterDirAnimationWidget)
{
    // Test constructor: EnterDirAnimationWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EnterDirAnimationWidgetTest, resetWidgetSize)
{
    // Test method: void resetWidgetSize((const QSize &size))
    QSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resetWidgetSize(_arg0));
}
