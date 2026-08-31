// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_autoactivatewindow_1.cpp
 * @brief Unit tests for AutoActivateWindow methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/autoactivatewindow.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class AutoActivateWindowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AutoActivateWindow();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AutoActivateWindow *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AutoActivateWindowTest, setWatched)
{
    // Test setter: void setWatched((QWidget *win))
    EXPECT_NO_FATAL_FAILURE(obj->setWatched(nullptr));
}

TEST_F(AutoActivateWindowTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(AutoActivateWindowTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
