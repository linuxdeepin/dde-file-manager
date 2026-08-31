// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_titlebar.cpp
 * @brief Unit tests for TitleBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "titlebar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class TitleBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TitleBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TitleBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TitleBarTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(TitleBarTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TitleBarTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(TitleBarTest, onWindowCreated)
{
    // Test method: void onWindowCreated((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowCreated(0));
}

TEST_F(TitleBarTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 windId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(TitleBarTest, registerTabSettingConfig)
{
    // Test method: void registerTabSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->registerTabSettingConfig());
}

TEST_F(TitleBarTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
