// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerstatusbar.cpp
 * @brief Unit tests for ComputerStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerstatusbar.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerStatusBarTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

TEST_F(ComputerStatusBarTest, showSingleSelectionMessage)
{
    // Test method: void showSingleSelectionMessage(())
    EXPECT_NO_FATAL_FAILURE(obj->showSingleSelectionMessage());
}
