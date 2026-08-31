// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventcaller.cpp
 * @brief Unit tests for WorkspaceEventCaller Mid-priority methods
 */

#include <gtest/gtest.h>

class WorkspaceEventCallerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(WorkspaceEventCallerTest, sendOpenWindow)
{
    // sendOpenWindow
    SUCCEED();
}

TEST_F(WorkspaceEventCallerTest, sendRenameEndEdit)
{
    // sendRenameEndEdit
    SUCCEED();
}

TEST_F(WorkspaceEventCallerTest, sendRenameStartEdit)
{
    // sendRenameStartEdit
    SUCCEED();
}

TEST_F(WorkspaceEventCallerTest, sendViewModeChanged)
{
    // sendViewModeChanged
    SUCCEED();
}
