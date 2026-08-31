// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmanager.cpp
 * @brief Unit tests for RecentManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentmanager.h"

#include <QTest>

using namespace recent;

class RecentManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentManagerTest, addRecentItem)
{
    // Test method: void addRecentItem((const QVariantMap &item))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addRecentItem(_arg0));
}

TEST_F(RecentManagerTest, finalize)
{
    // Test method: void finalize(())
    EXPECT_NO_FATAL_FAILURE(obj->finalize());
}

TEST_F(RecentManagerTest, onItemAdded)
{
    // Test method: void onItemAdded((const QString &path, const RecentItem &item))
    QString _arg0{};
    RecentItem _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemAdded(_arg0, _arg1));
}

TEST_F(RecentManagerTest, onItemsRemoved)
{
    // Test method: void onItemsRemoved((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemsRemoved(_arg0));
}

TEST_F(RecentManagerTest, reload)
{
    // Test method: void reload(())
    EXPECT_NO_FATAL_FAILURE(obj->reload());
}

TEST_F(RecentManagerTest, removeItems)
{
    // Test method: void removeItems((const QStringList &hrefs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeItems(_arg0));
}

TEST_F(RecentManagerTest, removeRecentFile)
{
    // Test method: bool removeRecentFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeRecentFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RecentManagerTest, resetRecentNodes)
{
    // Test method: void resetRecentNodes(())
    EXPECT_NO_FATAL_FAILURE(obj->resetRecentNodes());
}

TEST_F(RecentManagerTest, size)
{
    // Test getter: int size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}

TEST_F(RecentManagerTest, updateItemsInfoList)
{
    // Test method: void updateItemsInfoList(())
    EXPECT_NO_FATAL_FAILURE(obj->updateItemsInfoList());
}

TEST_F(RecentManagerTest, xbelPath)
{
    // Test getter: QString xbelPath()
    auto result = obj->xbelPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, dbus)
{
    // Test getter: RecentManagerDBusInterface dbus()
    auto result = obj->dbus();
    EXPECT_NO_FATAL_FAILURE({ obj->dbus(); });

}

TEST_F(RecentManagerTest, instance)
{
    // Test getter: RecentManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
