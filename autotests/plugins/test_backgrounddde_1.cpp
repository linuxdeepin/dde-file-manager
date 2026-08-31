// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgrounddde_1.cpp
 * @brief Unit tests for BackgroundDDE methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "backgrounddde.h"

#include <QTest>

using namespace ddplugin_background;

class BackgroundDDETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BackgroundDDE();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BackgroundDDE *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BackgroundDDETest, getBackgroundFromDDE)
{
    // Test method: QString getBackgroundFromDDE((const QString &screen))
    QString _arg0{};
    auto result = obj->getBackgroundFromDDE(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundDDETest, getDefaultBackground)
{
    // Test getter: QString getDefaultBackground()
    auto result = obj->getDefaultBackground();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BackgroundDDETest, onAppearanceValueChanged)
{
    // Test method: void onAppearanceValueChanged((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAppearanceValueChanged(_arg0));
}

TEST_F(BackgroundDDETest, BackgroundDDE_Destructor)
{
    // Test method:  ~BackgroundDDE(())
    EXPECT_NO_FATAL_FAILURE({ BackgroundDDE *tmp = new BackgroundDDE(); delete tmp; });
}
