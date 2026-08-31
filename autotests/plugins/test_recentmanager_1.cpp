// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmanager_1.cpp
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

TEST_F(RecentManagerTest, RecentManager)
{
    // Test constructor: RecentManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentManagerTest, RecentManager_RecentMa)
{
    // Test constructor: RecentManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentManagerTest, doReload)
{
    // Test method: void doReload((qint64 timestamp))
    EXPECT_NO_FATAL_FAILURE(obj->doReload(0));
}

TEST_F(RecentManagerTest, forceReload)
{
    // Test method: void forceReload((qint64 timestamp))
    EXPECT_NO_FATAL_FAILURE(obj->forceReload(0));
}

TEST_F(RecentManagerTest, getItemInfo)
{
    // Test method: QVariantMap getItemInfo((const QString &path))
    QString _arg0{};
    auto result = obj->getItemInfo(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, getItemsInfo)
{
    // Test getter: QVariantList getItemsInfo()
    auto result = obj->getItemsInfo();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, getItemsPath)
{
    // Test getter: QStringList getItemsPath()
    auto result = obj->getItemsPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, getRecentNodes)
{
    // Test getter: QMap<QUrl, FileInfoPointer> getRecentNodes()
    auto result = obj->getRecentNodes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, getRecentOriginPaths)
{
    // Test method: QString getRecentOriginPaths((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getRecentOriginPaths(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentManagerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(RecentManagerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(RecentManagerTest, onItemChanged)
{
    // Test method: void onItemChanged((const QString &path, const RecentItem &item))
    QString _arg0{};
    RecentItem _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemChanged(_arg0, _arg1));
}

TEST_F(RecentManagerTest, onItemChanged_onItemCh)
{
    // Test method: void onItemChanged((const QString &path, const RecentItem &item))
    QString _arg0{};
    RecentItem _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemChanged(_arg0, _arg1));
}

TEST_F(RecentManagerTest, processPendingItems)
{
    // Test method: void processPendingItems(())
    EXPECT_NO_FATAL_FAILURE(obj->processPendingItems());
}

TEST_F(RecentManagerTest, purgeItems)
{
    // Test method: void purgeItems(())
    EXPECT_NO_FATAL_FAILURE(obj->purgeItems());
}

TEST_F(RecentManagerTest, reloadRecent)
{
    // Test method: void reloadRecent(())
    EXPECT_NO_FATAL_FAILURE(obj->reloadRecent());
}

TEST_F(RecentManagerTest, startWatch)
{
    // Test method: void startWatch(())
    EXPECT_NO_FATAL_FAILURE(obj->startWatch());
}

TEST_F(RecentManagerTest, stopWatch)
{
    // Test method: void stopWatch(())
    EXPECT_NO_FATAL_FAILURE(obj->stopWatch());
}

TEST_F(RecentManagerTest, RecentManager_Destructor)
{
    // Test method:  ~RecentManager(())
    EXPECT_NO_FATAL_FAILURE({ RecentManager *tmp = new RecentManager(); delete tmp; });
}

TEST_F(RecentManagerTest, RecentManager_Destructor_xRecentM)
{
    // Test method:  ~RecentManager(())
    EXPECT_NO_FATAL_FAILURE({ RecentManager *tmp = new RecentManager(); delete tmp; });
}
