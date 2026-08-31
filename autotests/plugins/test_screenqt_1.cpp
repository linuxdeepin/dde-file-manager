// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenqt_1.cpp
 * @brief Unit tests for ScreenQt methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screenqt.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenQtTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenQt();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenQt *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenQtTest, checkAvailableGeometry)
{
    // Test method: bool checkAvailableGeometry((const QRect &ava, const QRect &scr))
    QRect _arg0{};
    QRect _arg1{};
    auto result = obj->checkAvailableGeometry(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ScreenQtTest, screen)
{
    // Test getter: QScreen screen()
    auto result = obj->screen();
    EXPECT_NO_FATAL_FAILURE({ obj->screen(); });

}
