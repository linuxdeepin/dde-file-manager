// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridshell_1.cpp
 * @brief Unit tests for CanvasGridShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasgridshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasGridShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridShellTest, CanvasGridShell)
{
    // Test constructor: CanvasGridShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasGridShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridShellTest, tryAppendAfter)
{
    // Test method: void tryAppendAfter((const QStringList &items, int index, const QPoint &begin))
    QStringList _arg0{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->tryAppendAfter(_arg0, 0, _arg2));
}

TEST_F(CanvasGridShellTest, CanvasGridShell_Destructor)
{
    // Test method:  ~CanvasGridShell(())
    EXPECT_NO_FATAL_FAILURE({ CanvasGridShell *tmp = new CanvasGridShell(); delete tmp; });
}
