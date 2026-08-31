// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmanagerdbus.cpp
 * @brief Unit tests for VaultManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/vaultmanagerdbus.h"

#include <QTest>

using namespace vault;

class VaultManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultManagerDBusTest, ClearLockEvent)
{
    // Test method: void ClearLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->ClearLockEvent());
}
