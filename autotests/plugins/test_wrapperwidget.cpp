// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wrapperwidget.cpp
 * @brief Unit tests for WrapperWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpaperitem.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WrapperWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WrapperWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WrapperWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WrapperWidgetTest, opacity)
{
    // Test getter: qreal opacity()
    auto result = obj->opacity();
    EXPECT_EQ(result, 0.0);

}
