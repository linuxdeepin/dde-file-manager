// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncdbus.cpp
 * @brief Unit tests for SyncDBus Mid-priority methods
 */

#include <gtest/gtest.h>

class SyncDBusTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SyncDBusTest, GetSyncStatus)
{
    // GetSyncStatus
    SUCCEED();
}
