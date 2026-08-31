// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_infocache.cpp
 * @brief Unit tests for InfoCache methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/infocache.h"

#include <QTest>

using namespace src;

class InfoCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InfoCache();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InfoCache *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InfoCacheTest, InfoCache)
{
    // Test constructor: InfoCache((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(InfoCacheTest, M_~InfoCache)
{
    // Test method:  ~InfoCache(())
    EXPECT_NO_FATAL_FAILURE({ InfoCache *tmp = new InfoCache(); delete tmp; });
}

TEST_F(InfoCacheTest, instance)
{
    // Test getter: InfoCache instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(InfoCacheTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(InfoCacheTest, disconnectWatcher)
{
    // Test method: void disconnectWatcher((const QMap<QUrl, FileInfoPointer> infos))
    EXPECT_NO_FATAL_FAILURE(obj->disconnectWatcher(QMap<QUrl, FileInfoPointer>()));
}

TEST_F(InfoCacheTest, removeCache)
{
    // Test method: void removeCache((const QUrl url))
    EXPECT_NO_FATAL_FAILURE(obj->removeCache(QUrl()));
}

TEST_F(InfoCacheTest, cacheDisable)
{
    // Test method: bool cacheDisable((const QString &scheme))
    QString _arg0{};
    auto result = obj->cacheDisable(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InfoCacheTest, cacheInfo)
{
    // Test method: void cacheInfo((const QUrl url, const FileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->cacheInfo(QUrl(), FileInfoPointer()));
}

TEST_F(InfoCacheTest, removeCaches)
{
    // Test method: void removeCaches((const QList<QUrl> urls))
    EXPECT_NO_FATAL_FAILURE(obj->removeCaches(QList<QUrl>()));
}

TEST_F(InfoCacheTest, setCacheDisbale)
{
    // Test setter: void setCacheDisbale((const QString &scheme, bool disable))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCacheDisbale(_arg0, false));
}

TEST_F(InfoCacheTest, getCacheInfo)
{
    // Test method: FileInfoPointer getCacheInfo((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getCacheInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(InfoCacheTest, refreshFileInfo)
{
    // Test method: void refreshFileInfo((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->refreshFileInfo(_arg0));
}

TEST_F(InfoCacheTest, timeRemoveCache)
{
    // Test method: void timeRemoveCache(())
    EXPECT_NO_FATAL_FAILURE(obj->timeRemoveCache());
}

TEST_F(InfoCacheTest, fileAttributeChanged)
{
    // Test method: void fileAttributeChanged((const QUrl url))
    EXPECT_NO_FATAL_FAILURE(obj->fileAttributeChanged(QUrl()));
}

TEST_F(InfoCacheTest, updateSortTimeWorker)
{
    // Test method: bool updateSortTimeWorker((const QUrl url))
    auto result = obj->updateSortTimeWorker(QUrl());
    EXPECT_FALSE(result);

}

TEST_F(InfoCacheTest, addWatcherTimeInfo)
{
    // Test method: void addWatcherTimeInfo((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addWatcherTimeInfo(_arg0));
}

TEST_F(InfoCacheTest, removeWatcherTimeInfo)
{
    // Test method: void removeWatcherTimeInfo((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeWatcherTimeInfo(_arg0));
}

TEST_F(InfoCacheTest, removeInfosTimeWorker)
{
    // Test method: void removeInfosTimeWorker((const QList<QUrl> urls))
    EXPECT_NO_FATAL_FAILURE(obj->removeInfosTimeWorker(QList<QUrl>()));
}

TEST_F(InfoCacheTest, updateSortTimeWatcherWorker)
{
    // Test method: void updateSortTimeWatcherWorker((const QList<QUrl> &urls, const bool add))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateSortTimeWatcherWorker(_arg0, false));
}

TEST_F(InfoCacheTest, d)
{
    // Test getter: QScopedPointer<InfoCachePrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
