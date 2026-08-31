// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectiondataprovider.cpp
 * @brief Unit tests for CollectionDataProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/collectiondataprovider.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionDataProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionDataProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionDataProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionDataProviderTest, checkPreItem)
{
    // Test method: bool checkPreItem((const QUrl &url, QString &key, int &index))
    QUrl _arg0{};
    QString _arg1{};
    int _arg2{};
    auto result = obj->checkPreItem(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(CollectionDataProviderTest, contains)
{
    // Test method: bool contains((const QString &key, const QUrl &url))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->contains(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CollectionDataProviderTest, items)
{
    // Test method: QList<QUrl> items((const QString &key))
    QString _arg0{};
    auto result = obj->items(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionDataProviderTest, key)
{
    // Test method: QString key((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->key(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionDataProviderTest, keys)
{
    // Test getter: QList<QString> keys()
    auto result = obj->keys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionDataProviderTest, name)
{
    // Test method: QString name((const QString &key))
    QString _arg0{};
    auto result = obj->name(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionDataProviderTest, sorted)
{
    // Test method: bool sorted((const QString &key, const QList<QUrl> &urls))
    QString _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->sorted(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CollectionDataProviderTest, takePreItem)
{
    // Test method: bool takePreItem((const QUrl &url, QString &key, int &index))
    QUrl _arg0{};
    QString _arg1{};
    int _arg2{};
    auto result = obj->takePreItem(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(CollectionDataProviderTest, CollectionDataProvider)
{
    // Test constructor: CollectionDataProvider((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
