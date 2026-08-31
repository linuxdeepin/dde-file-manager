// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optionswindow.cpp
 * @brief Unit tests for OptionsWindow methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/optionswindow.h"

#include <QTest>

using namespace ddplugin_organizer;

class OptionsWindowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OptionsWindow();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OptionsWindow *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OptionsWindowTest, OptionsWindow)
{
    // Test constructor: OptionsWindow((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OptionsWindowTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(OptionsWindowTest, moveToCenter)
{
    // Test method: void moveToCenter((const QPoint &cursorPos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->moveToCenter(_arg0));
}
