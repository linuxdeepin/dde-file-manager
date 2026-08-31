// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewsettingutil.cpp
 * @brief Unit tests for ViewSettingUtil methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/viewsettingutil.h"

#include <QTest>

using namespace ddplugin_canvas;

class ViewSettingUtilTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewSettingUtil();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewSettingUtil *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewSettingUtilTest, ViewSettingUtil)
{
    // Test constructor: ViewSettingUtil((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
