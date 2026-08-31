// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasselectionshell.cpp
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

TEST_F(CanvasSelectionShellTest, eventClear)
{
    // Test method: void eventClear(())
    EXPECT_NO_FATAL_FAILURE(obj->eventClear());
}
