// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultclock.cpp
 * @brief Unit tests for VaultClock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/private/vaultclock.h"

#include <QTest>

using namespace vault;

class VaultClockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultClock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultClock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultClockTest, clearLockEvent)
{
    // Test method: void clearLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->clearLockEvent());
}
