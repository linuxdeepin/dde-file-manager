// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncdbus.cpp
 * @brief Unit tests for SyncDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "syncdbus.h"

#include <QTest>

using namespace core;

class SyncDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncDBusTest, GetSyncStatus)
{
    // Test getter: QVariantMap GetSyncStatus()
    auto result = obj->GetSyncStatus();
    EXPECT_TRUE(result.isEmpty());

}
