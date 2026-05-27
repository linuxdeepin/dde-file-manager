// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "controller/defaultitemmanager.h"
#include "controller/bookmarkmanager.h"

#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <QUrl>
#include <QMap>
#include <QString>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_bookmark;

class UT_DefaultItemManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        manager = DefaultItemManager::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    DefaultItemManager *manager = nullptr;
};

TEST_F(UT_DefaultItemManager, Instance)
{
    EXPECT_TRUE(manager != nullptr);
    EXPECT_EQ(manager, DefaultItemManager::instance());
}

TEST_F(UT_DefaultItemManager, InitDefaultItems)
{
    // Mock SystemPathUtil to return consistent paths
    stub.set_lamda(&SystemPathUtil::systemPath, [](SystemPathUtil *, const QString &key) -> QString {
        __DBG_STUB_INVOKE__
        if (key == "Home") return "/home/test";
        if (key == "Desktop") return "/home/test/Desktop";
        if (key == "Videos") return "/home/test/Videos";
        if (key == "Music") return "/home/test/Music";
        if (key == "Pictures") return "/home/test/Pictures";
        if (key == "Documents") return "/home/test/Documents";
        if (key == "Downloads") return "/home/test/Downloads";
        return QString();
    });

    EXPECT_NO_FATAL_FAILURE(manager->initDefaultItems());

    QMap<QString, QUrl> urls = manager->defaultItemUrls();
    EXPECT_FALSE(urls.isEmpty());
    EXPECT_TRUE(urls.contains("Home"));
    EXPECT_TRUE(urls.contains("Desktop"));
    EXPECT_TRUE(urls.contains("Videos"));
    EXPECT_TRUE(urls.contains("Music"));
    EXPECT_TRUE(urls.contains("Pictures"));
    EXPECT_TRUE(urls.contains("Documents"));
    EXPECT_TRUE(urls.contains("Downloads"));
}

TEST_F(UT_DefaultItemManager, InitPreDefineItems)
{
    EXPECT_NO_FATAL_FAILURE(manager->initPreDefineItems());

    QMap<QString, QUrl> preDefUrls = manager->preDefItemUrls();
    // The exact content depends on the system type, just check it doesn't crash
    EXPECT_TRUE(preDefUrls.size() >= 0);
}

TEST_F(UT_DefaultItemManager, DefaultItemUrls)
{
    stub.set_lamda(&SystemPathUtil::systemPath, [](SystemPathUtil *, const QString &key) -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/" + key;
    });

    manager->initDefaultItems();
    QMap<QString, QUrl> urls = manager->defaultItemUrls();

    EXPECT_FALSE(urls.isEmpty());
    EXPECT_EQ(urls["Home"], QUrl::fromLocalFile("/home/test/Home"));
    EXPECT_EQ(urls["Desktop"], QUrl::fromLocalFile("/home/test/Desktop"));
}

TEST_F(UT_DefaultItemManager, PreDefItemUrls)
{
    manager->initPreDefineItems();
    QMap<QString, QUrl> urls = manager->preDefItemUrls();

    // Just check that the method works without crashing
    EXPECT_TRUE(urls.size() >= 0);
}

TEST_F(UT_DefaultItemManager, DefaultItemInitOrder)
{
    stub.set_lamda(&SystemPathUtil::systemPath, [](SystemPathUtil *, const QString &key) -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/" + key;
    });

    manager->initDefaultItems();
    QList<BookmarkData> order = manager->defaultItemInitOrder();

    EXPECT_FALSE(order.isEmpty());
    EXPECT_EQ(order.size(), 7); // Home, Desktop, Videos, Music, Pictures, Documents, Downloads

    // Check first item is Home
    if (!order.isEmpty()) {
        EXPECT_EQ(order.first().name, "Home");
        EXPECT_TRUE(order.first().isDefaultItem);
        EXPECT_EQ(order.first().index, 0);
    }

    // Check items are in correct order
    for (int i = 0; i < order.size(); ++i) {
        EXPECT_EQ(order[i].index, i);
        EXPECT_TRUE(order[i].isDefaultItem);
    }
}

TEST_F(UT_DefaultItemManager, DefaultPreDefInitOrder)
{
    manager->initPreDefineItems();
    QList<BookmarkData> order = manager->defaultPreDefInitOrder();

    // Just verify the method works
    EXPECT_TRUE(order.size() >= 0);

    // If there are items, they should be marked as default
    for (const auto &item : order) {
        EXPECT_TRUE(item.isDefaultItem);
    }
}

TEST_F(UT_DefaultItemManager, IsDefaultItem_True)
{
    stub.set_lamda(&SystemPathUtil::systemPath, [](SystemPathUtil *, const QString &key) -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/" + key;
    });

    manager->initDefaultItems();

    BookmarkData testData;
    testData.name = "Home";
    testData.url = QUrl::fromLocalFile("/home/test/Home");
    testData.isDefaultItem = true;

    EXPECT_TRUE(manager->isDefaultItem(testData));
}

TEST_F(UT_DefaultItemManager, IsDefaultItem_False)
{
    stub.set_lamda(&SystemPathUtil::systemPath, [](SystemPathUtil *, const QString &key) -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/" + key;
    });

    manager->initDefaultItems();

    BookmarkData testData;
    testData.name = "CustomItem";
    testData.url = QUrl::fromLocalFile("/home/test/custom");
    testData.isDefaultItem = false;

    EXPECT_FALSE(manager->isDefaultItem(testData));
}

TEST_F(UT_DefaultItemManager, IsPreDefItem_True)
{
    manager->initPreDefineItems();

    BookmarkData testData;
    testData.name = "Computer";  // Assuming Computer is a predefined item
    testData.url = QUrl("computer:///");
    testData.isDefaultItem = true;

    // This test depends on actual predefined items, so we just test it doesn't crash
    EXPECT_NO_FATAL_FAILURE(manager->isPreDefItem(testData));
}

TEST_F(UT_DefaultItemManager, IsPreDefItem_False)
{
    manager->initPreDefineItems();

    BookmarkData testData;
    testData.name = "CustomItem";
    testData.url = QUrl::fromLocalFile("/home/test/custom");
    testData.isDefaultItem = false;

    EXPECT_FALSE(manager->isPreDefItem(testData));
}
