// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmanagerdbus_1.cpp
 * @brief Unit tests for RecentManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentmanagerdbus.h"

#include <QTest>

using namespace recent;

class RecentManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentManagerDBusTest, AddItem)
{
    // Test method: void AddItem((const QVariantMap &item))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->AddItem(_arg0));
}

TEST_F(RecentManagerDBusTest, GetItemInfo)
{
    // Test method: QVariantMap GetItemInfo((const QString &path))
    QString _arg0{};
    auto result = obj->GetItemInfo(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerDBusTest, GetItemsInfo)
{
    // Test getter: QVariantList GetItemsInfo()
    auto result = obj->GetItemsInfo();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerDBusTest, GetItemsPath)
{
    // Test getter: QStringList GetItemsPath()
    auto result = obj->GetItemsPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerDBusTest, PurgeItems)
{
    // Test method: void PurgeItems(())
    EXPECT_NO_FATAL_FAILURE(obj->PurgeItems());
}

TEST_F(RecentManagerDBusTest, Reload)
{
    // Test getter: qint64 Reload()
    auto result = obj->Reload();
    EXPECT_EQ(result, 0);

}

TEST_F(RecentManagerDBusTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}
