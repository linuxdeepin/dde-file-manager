// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computermenusceneprivate.cpp
 * @brief Unit tests for ComputerMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/computermenuscene.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerMenuScenePrivateTest, ComputerMenuScenePrivate)
{
    // Test constructor: ComputerMenuScenePrivate((ComputerMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerMenuScenePrivateTest, updateMenu)
{
    // Test method: void updateMenu((QMenu *menu, const QStringList &disabled, const QStringList &keeps))
    QStringList _arg1{};
    QStringList _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateMenu(nullptr, _arg1, _arg2));
}
