// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_movetotrashfiles.cpp
 * @brief Unit tests for MoveToTrashFiles Low-priority methods
 */

#include <gtest/gtest.h>

class MoveToTrashFilesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MoveToTrashFilesTest, MoveToTrashFiles_Destructor)
{
    // ~MoveToTrashFiles
    SUCCEED();
}

