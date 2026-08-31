// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_synctask_1.cpp
 * @brief Unit tests for SyncTask methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "syncdbus.h"

#include <QTest>

using namespace core;

class SyncTaskTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncTask();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncTask *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncTaskTest, SyncTask)
{
    // Test constructor: SyncTask((int taskId, const QString &path, const QVariantMap &options, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SyncTaskTest, errorMessage)
{
    // Test getter: QString errorMessage()
    auto result = obj->errorMessage();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SyncTaskTest, isSuccessful)
{
    // Test bool getter: isSuccessful()
    bool result = obj->isSuccessful();
    EXPECT_FALSE(result);

}

TEST_F(SyncTaskTest, taskId)
{
    // Test getter: int taskId()
    auto result = obj->taskId();
    EXPECT_EQ(result, 0);

}
