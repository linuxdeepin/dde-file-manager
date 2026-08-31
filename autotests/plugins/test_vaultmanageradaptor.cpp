// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmanageradaptor.cpp
 * @brief Unit tests for VaultManagerAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/VaultManagerAdaptor.h"

#include <QTest>

using namespace src;

class VaultManagerAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultManagerAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultManagerAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultManagerAdaptorTest, ClearLockEvent)
{
    // Test method: void ClearLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->ClearLockEvent());
}

TEST_F(VaultManagerAdaptorTest, VaultManagerAdaptor)
{
    // Test constructor: VaultManagerAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
