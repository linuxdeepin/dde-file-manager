// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncdbus_1.cpp
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

TEST_F(SyncDBusTest, SyncDBus)
{
    // Test constructor: SyncDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SyncDBusTest, SyncFS)
{
    // Test method: int SyncFS((const QString &path, const QVariantMap &options))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->SyncFS(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(SyncDBusTest, generateTaskId)
{
    // Test getter: int generateTaskId()
    auto result = obj->generateTaskId();
    EXPECT_EQ(result, 0);

}

TEST_F(SyncDBusTest, onSyncTaskCompleted)
{
    // Test method: void onSyncTaskCompleted((SyncTask *task))
    EXPECT_NO_FATAL_FAILURE(obj->onSyncTaskCompleted(nullptr));
}

TEST_F(SyncDBusTest, SyncDBus_Destructor)
{
    // Test method:  ~SyncDBus(())
    EXPECT_NO_FATAL_FAILURE({ SyncDBus *tmp = new SyncDBus(); delete tmp; });
}
