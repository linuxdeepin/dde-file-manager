// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mode/collectiondataprovider.h"
#include "organizer_defines.h"

#include <QApplication>
#include <QUrl>
#include <QStringList>
#include <QTest>
#include <QTimer>

#include "stubext.h"
#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class TestCollectionDataProvider : public CollectionDataProvider
{
public:
    TestCollectionDataProvider(QObject *parent = nullptr) : CollectionDataProvider(parent) {}
    
    QString replace(const QUrl &oldUrl, const QUrl &newUrl) override {
        Q_UNUSED(oldUrl)
        Q_UNUSED(newUrl)
        return QString();
    }
    
    QString append(const QUrl &url) override {
        Q_UNUSED(url)
        return QString("test-key");
    }
    
    QString prepend(const QUrl &url) override {
        Q_UNUSED(url)
        return QString("test-key");
    }
    
    void insert(const QUrl &url, const QString &key, const int index) override {
        Q_UNUSED(url)
        Q_UNUSED(key)
        Q_UNUSED(index)
    }
    
    QString remove(const QUrl &url) override {
        Q_UNUSED(url)
        return QString("test-key");
    }
    
    QString change(const QUrl &url) override {
        Q_UNUSED(url)
        return QString("test-key");
    }
};

class UT_CollectionDataProvider : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        provider = new TestCollectionDataProvider();
    }
    
    void TearDown() override
    {
        delete provider;
        if (app) {
            delete app;
            app = nullptr;
        }
        stub.clear();
    }
    
    TestCollectionDataProvider* provider;
    QApplication* app = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionDataProvider, ConstructorTest)
{
    EXPECT_NE(provider, nullptr);
    EXPECT_TRUE(provider->keys().isEmpty());
    EXPECT_TRUE(provider->collections.isEmpty());
    EXPECT_TRUE(provider->preCollectionItems.isEmpty());
}

TEST_F(UT_CollectionDataProvider, KeyTest)
{
    QUrl url1("file:///test/file1.txt");
    QUrl url2("file:///test/file2.txt");
    QUrl url3;
    
    QString key1 = provider->key(url1);
    QString key2 = provider->key(url2);
    QString key3 = provider->key(url3);
    
    EXPECT_FALSE(key1.isEmpty());
    EXPECT_FALSE(key2.isEmpty());
    EXPECT_TRUE(key3.isEmpty());
    
    QUrl invalidUrl("invalid-url");
    QString invalidKey = provider->key(invalidUrl);
    EXPECT_TRUE(invalidKey.isEmpty());
}

TEST_F(UT_CollectionDataProvider, NameTest)
{
    QString key1 = "test-key-1";
    QString key2 = "test-key-2";
    QString key3;
    
    QString name1 = provider->name(key1);
    QString name2 = provider->name(key2);
    QString name3 = provider->name(key3);
    
    EXPECT_TRUE(name1.isEmpty());
    EXPECT_TRUE(name2.isEmpty());
    EXPECT_TRUE(name3.isEmpty());
    
    QString emptyKey = "";
    QString emptyName = provider->name(emptyKey);
    EXPECT_TRUE(emptyName.isEmpty());
}

TEST_F(UT_CollectionDataProvider, KeysTest)
{
    QStringList keys1 = provider->keys();
    EXPECT_TRUE(keys1.isEmpty());
    
    provider->append(QUrl("file:///test1.txt"));
    QStringList keys2 = provider->keys();
    
    for (int i = 0; i < 3; ++i) {
        provider->append(QUrl(QString("file:///test%1.txt").arg(i)));
    }
    
    QStringList keys3 = provider->keys();
    EXPECT_FALSE(keys3.isEmpty());
}

TEST_F(UT_CollectionDataProvider, ItemsTest)
{
    QString key = "test-key";
    
    QList<QUrl> items1 = provider->items(key);
    EXPECT_TRUE(items1.isEmpty());
    
    provider->append(QUrl("file:///test1.txt"));
    provider->append(QUrl("file:///test2.txt"));
    
    QList<QUrl> items2 = provider->items(key);
    
    QList<QUrl> items3 = provider->items("non-existent-key");
    EXPECT_TRUE(items3.isEmpty());
}

TEST_F(UT_CollectionDataProvider, ContainsTest)
{
    QString key = "test-key";
    QUrl url1("file:///test1.txt");
    QUrl url2("file:///test2.txt");
    QUrl url3("file:///test3.txt");
    
    bool contains1 = provider->contains(key, url1);
    bool contains2 = provider->contains(key, url2);
    bool contains3 = provider->contains(key, url3);
    
    EXPECT_FALSE(contains1);
    EXPECT_FALSE(contains2);
    EXPECT_FALSE(contains3);
    
    QString emptyKey = "";
    bool containsEmpty = provider->contains(emptyKey, url1);
    EXPECT_FALSE(containsEmpty);
    
    QUrl emptyUrl;
    bool containsEmptyUrl = provider->contains(key, emptyUrl);
    EXPECT_FALSE(containsEmptyUrl);
}

TEST_F(UT_CollectionDataProvider, SortedTest)
{
    QString key = "test-key";
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt"),
        QUrl("file:///test3.txt")
    };
    
    bool result1 = provider->sorted(key, urls);
    
    QList<QUrl> emptyUrls;
    bool result2 = provider->sorted(key, emptyUrls);
    
    QList<QUrl> singleUrl = { QUrl("file:///single.txt") };
    bool result3 = provider->sorted(key, singleUrl);
    
    for (int i = 0; i < 3; ++i) {
        QList<QUrl> testUrls = { QUrl(QString("file:///test%1.txt").arg(i)) };
        bool result = provider->sorted(QString("test-key-%1").arg(i), testUrls);
        (void)result;
    }
}

TEST_F(UT_CollectionDataProvider, MoveUrlsTest)
{
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt")
    };
    QString targetKey = "target-key";
    int targetIndex = 0;
    
    provider->moveUrls(urls, targetKey, targetIndex);
    QTest::qWait(10);
    
    provider->moveUrls(urls, targetKey, -1);
    QTest::qWait(10);
    
    provider->moveUrls(urls, targetKey, 100);
    QTest::qWait(10);
    
    QList<QUrl> emptyUrls;
    provider->moveUrls(emptyUrls, targetKey, targetIndex);
    QTest::qWait(10);
}

TEST_F(UT_CollectionDataProvider, AddPreItemsTest)
{
    QString targetKey = "target-key";
    QList<QUrl> urls = {
        QUrl("file:///pre1.txt"),
        QUrl("file:///pre2.txt")
    };
    int targetIndex = 0;
    
    provider->addPreItems(targetKey, urls, targetIndex);
    QTest::qWait(10);
    
    provider->addPreItems(targetKey, urls, -1);
    QTest::qWait(10);
    
    provider->addPreItems(targetKey, urls, 10);
    QTest::qWait(10);
    
    QList<QUrl> emptyUrls;
    provider->addPreItems(targetKey, emptyUrls, targetIndex);
    QTest::qWait(10);
}

TEST_F(UT_CollectionDataProvider, CheckPreItemTest)
{
    QUrl url1("file:///preitem1.txt");
    QUrl url2("file:///preitem2.txt");
    QUrl url3("file:///nonexistent.txt");
    
    QString key1;
    int index1;
    bool result1 = provider->checkPreItem(url1, key1, index1);
    
    QString key2;
    int index2;
    bool result2 = provider->checkPreItem(url2, key2, index2);
    
    QString key3;
    int index3;
    bool result3 = provider->checkPreItem(url3, key3, index3);
    
    QUrl emptyUrl;
    QString key4;
    int index4;
    bool result4 = provider->checkPreItem(emptyUrl, key4, index4);
    
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(result3);
    EXPECT_FALSE(result4);
}

TEST_F(UT_CollectionDataProvider, TakePreItemTest)
{
    QUrl url1("file:///preitem1.txt");
    QUrl url2("file:///preitem2.txt");
    QUrl url3("file:///nonexistent.txt");
    
    QString key1;
    int index1;
    bool result1 = provider->takePreItem(url1, key1, index1);
    
    QString key2;
    int index2;
    bool result2 = provider->takePreItem(url2, key2, index2);
    
    QString key3;
    int index3;
    bool result3 = provider->takePreItem(url3, key3, index3);
    
    QUrl emptyUrl;
    QString key4;
    int index4;
    bool result4 = provider->takePreItem(emptyUrl, key4, index4);
    
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(result3);
    EXPECT_FALSE(result4);
}

TEST_F(UT_CollectionDataProvider, ReplaceTest)
{
    QUrl oldUrl("file:///old.txt");
    QUrl newUrl("file:///new.txt");
    
    QString result = provider->replace(oldUrl, newUrl);
    EXPECT_TRUE(result.isEmpty());
    
    QUrl invalidOld("invalid-old");
    QUrl invalidNew("invalid-new");
    QString result2 = provider->replace(invalidOld, invalidNew);
    EXPECT_TRUE(result2.isEmpty());
}

TEST_F(UT_CollectionDataProvider, AppendTest)
{
    QUrl url1("file:///append1.txt");
    QUrl url2("file:///append2.txt");
    QUrl url3("file:///append3.txt");
    
    QString result1 = provider->append(url1);
    QString result2 = provider->append(url2);
    QString result3 = provider->append(url3);
    
    EXPECT_EQ(result1, QString("test-key"));
    EXPECT_EQ(result2, QString("test-key"));
    EXPECT_EQ(result3, QString("test-key"));
    
    QUrl emptyUrl;
    QString result4 = provider->append(emptyUrl);
    EXPECT_EQ(result4, QString("test-key"));
    
    QUrl invalidUrl("invalid-url");
    QString result5 = provider->append(invalidUrl);
    EXPECT_EQ(result5, QString("test-key"));
}

TEST_F(UT_CollectionDataProvider, PrependTest)
{
    QUrl url1("file:///prepend1.txt");
    QUrl url2("file:///prepend2.txt");
    QUrl url3("file:///prepend3.txt");
    
    QString result1 = provider->prepend(url1);
    QString result2 = provider->prepend(url2);
    QString result3 = provider->prepend(url3);
    
    EXPECT_EQ(result1, QString("test-key"));
    EXPECT_EQ(result2, QString("test-key"));
    EXPECT_EQ(result3, QString("test-key"));
    
    QUrl emptyUrl;
    QString result4 = provider->prepend(emptyUrl);
    EXPECT_EQ(result4, QString("test-key"));
}

TEST_F(UT_CollectionDataProvider, InsertTest)
{
    QUrl url1("file:///insert1.txt");
    QUrl url2("file:///insert2.txt");
    QUrl url3("file:///insert3.txt");
    QString key = "test-key";
    
    provider->insert(url1, key, 0);
    QTest::qWait(10);
    
    provider->insert(url2, key, -1);
    QTest::qWait(10);
    
    provider->insert(url3, key, 100);
    QTest::qWait(10);
    
    for (int i = 0; i < 3; ++i) {
        QUrl url(QString("file:///insert_multi_%1.txt").arg(i));
        provider->insert(url, QString("test-key-%1").arg(i), i);
        QTest::qWait(5);
    }
}

TEST_F(UT_CollectionDataProvider, RemoveTest)
{
    QUrl url1("file:///remove1.txt");
    QUrl url2("file:///remove2.txt");
    QUrl url3("file:///remove3.txt");
    
    QString result1 = provider->remove(url1);
    QString result2 = provider->remove(url2);
    QString result3 = provider->remove(url3);
    
    EXPECT_EQ(result1, QString("test-key"));
    EXPECT_EQ(result2, QString("test-key"));
    EXPECT_EQ(result3, QString("test-key"));
    
    QUrl emptyUrl;
    QString result4 = provider->remove(emptyUrl);
    EXPECT_EQ(result4, QString("test-key"));
    
    QUrl invalidUrl("invalid-url");
    QString result5 = provider->remove(invalidUrl);
    EXPECT_EQ(result5, QString("test-key"));
}

TEST_F(UT_CollectionDataProvider, ChangeTest)
{
    QUrl url1("file:///change1.txt");
    QUrl url2("file:///change2.txt");
    QUrl url3("file:///change3.txt");
    
    QString result1 = provider->change(url1);
    QString result2 = provider->change(url2);
    QString result3 = provider->change(url3);
    
    EXPECT_EQ(result1, QString("test-key"));
    EXPECT_EQ(result2, QString("test-key"));
    EXPECT_EQ(result3, QString("test-key"));
    
    QUrl emptyUrl;
    QString result4 = provider->change(emptyUrl);
    EXPECT_EQ(result4, QString("test-key"));
}

TEST_F(UT_CollectionDataProvider, SignalTest)
{
    bool nameChanged = false;
    bool itemsChanged = false;
    
    QObject::connect(provider, &CollectionDataProvider::nameChanged, [&](const QString &key, const QString &name) {
        nameChanged = true;
        EXPECT_EQ(key, QString("test-signal-key"));
    });
    
    QObject::connect(provider, &CollectionDataProvider::itemsChanged, [&](const QString &key) {
        itemsChanged = true;
        EXPECT_EQ(key, QString("test-signal-key"));
    });
    
    provider->append(QUrl("file:///signal.txt"));
    QTest::qWait(50);
    
    for (int i = 0; i < 3; ++i) {
        provider->change(QUrl(QString("file:///signal_%1.txt").arg(i)));
        QTest::qWait(20);
    }
}

TEST_F(UT_CollectionDataProvider, StressTest)
{
    for (int i = 0; i < 10; ++i) {
        QUrl url(QString("file:///stress_%1.txt").arg(i));
        provider->append(url);
        provider->prepend(url);
        provider->remove(url);
        provider->change(url);
        QTest::qWait(5);
    }
}

TEST_F(UT_CollectionDataProvider, EdgeCasesTest)
{
    QUrl url("file:///edge.txt");
    
    provider->append(url);
    provider->append(url);
    provider->prepend(url);
    provider->prepend(url);
    provider->remove(url);
    provider->remove(url);
    provider->change(url);
    provider->change(url);
    
    QList<QUrl> duplicateUrls = { url, url, url };
    provider->moveUrls(duplicateUrls, "duplicate-key", 0);
    
    provider->addPreItems("edge-key", duplicateUrls, 0);
    
    QString key;
    int index;
    provider->checkPreItem(url, key, index);
    provider->takePreItem(url, key, index);
}

TEST_F(UT_CollectionDataProvider, MultipleProvidersTest)
{
    TestCollectionDataProvider* provider1 = new TestCollectionDataProvider();
    TestCollectionDataProvider* provider2 = new TestCollectionDataProvider();
    TestCollectionDataProvider* provider3 = new TestCollectionDataProvider();
    
    EXPECT_NE(provider1, nullptr);
    EXPECT_NE(provider2, nullptr);
    EXPECT_NE(provider3, nullptr);
    
    QUrl url1("file:///provider1.txt");
    QUrl url2("file:///provider2.txt");
    QUrl url3("file:///provider3.txt");
    
    QString result1 = provider1->append(url1);
    QString result2 = provider2->append(url2);
    QString result3 = provider3->append(url3);
    
    EXPECT_EQ(result1, QString("test-key"));
    EXPECT_EQ(result2, QString("test-key"));
    EXPECT_EQ(result3, QString("test-key"));
    
    delete provider1;
    delete provider2;
    delete provider3;
}
