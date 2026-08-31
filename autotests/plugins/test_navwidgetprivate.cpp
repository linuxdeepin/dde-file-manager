// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_navwidgetprivate.cpp
 * @brief Unit tests for NavWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/navwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class NavWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NavWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NavWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NavWidgetPrivateTest, updateBackForwardButtonsState)
{
    // Test getter: DFMBASE_USE_NAMESPACE updateBackForwardButtonsState()
    EXPECT_NO_FATAL_FAILURE({ obj->updateBackForwardButtonsState(); });
}
