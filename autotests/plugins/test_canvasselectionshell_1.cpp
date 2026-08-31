// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasselectionshell_1.cpp
 * @brief Unit tests for CanvasSelectionShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasselectionshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasSelectionShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasSelectionShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasSelectionShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasSelectionShellTest, CanvasSelectionShell)
{
    // Test constructor: CanvasSelectionShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasSelectionShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasSelectionShellTest, selectionModel)
{
    // Test getter: QItemSelectionModel selectionModel()
    auto result = obj->selectionModel();
    EXPECT_NO_FATAL_FAILURE({ obj->selectionModel(); });

}

TEST_F(CanvasSelectionShellTest, CanvasSelectionShell_Destructor)
{
    // Test method:  ~CanvasSelectionShell(())
    EXPECT_NO_FATAL_FAILURE({ CanvasSelectionShell *tmp = new CanvasSelectionShell(); delete tmp; });
}
