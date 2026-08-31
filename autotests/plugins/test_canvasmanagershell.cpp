// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmanagershell.cpp
 * @brief Unit tests for CanvasManagerShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasmanagershell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasManagerShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasManagerShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasManagerShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerShellTest, CanvasManagerShell)
{
    // Test constructor: CanvasManagerShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasManagerShellTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasManagerShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasManagerShellTest, setIconLevel)
{
    // Test setter: void setIconLevel((const int level))
    EXPECT_NO_FATAL_FAILURE(obj->setIconLevel(0));
}

TEST_F(CanvasManagerShellTest, CanvasManagerShell_Destructor)
{
    // Test method:  ~CanvasManagerShell(())
    EXPECT_NO_FATAL_FAILURE({ CanvasManagerShell *tmp = new CanvasManagerShell(); delete tmp; });
}
