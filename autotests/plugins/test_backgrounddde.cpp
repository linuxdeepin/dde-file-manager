// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_backgrounddde.cpp
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

TEST_F(BackgroundDDETest, background)
{
    // Test method: QString background((const QString &screen))
    QString _arg0{};
    auto result = obj->background(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
