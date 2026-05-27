// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/sidebarinfocachemananger.h"
#include "dfmplugin_sidebar_global.h"

#include <dfm-base/utils/universalutils.h>

#include <QUrl>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE

class UT_SideBarInfoCacheMananger : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        manager = SideBarInfoCacheMananger::instance();
        ASSERT_NE(manager, nullptr);

        // Clear any existing cache for clean test state
        manager->clearLastSettingKey();
        manager->clearLastSettingBindingKey();
        manager->cacheInfoMap.clear();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    SideBarInfoCacheMananger *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarInfoCacheMananger, Instance)
{
    auto ins1 = SideBarInfoCacheMananger::instance();
    auto ins2 = SideBarInfoCacheMananger::instance();

    EXPECT_NE(ins1, nullptr);
    EXPECT_EQ(ins1, ins2);
}

TEST_F(UT_SideBarInfoCacheMananger, AddItemInfoCache_Success)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/test");
    info.group = DefaultGroup::kCommon;
    info.displayName = "Test";

    bool result = manager->addItemInfoCache(info);
    EXPECT_TRUE(result);

    EXPECT_TRUE(manager->contains(info));
    EXPECT_TRUE(manager->contains(info.url));
}

TEST_F(UT_SideBarInfoCacheMananger, AddItemInfoCache_Duplicate)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/duplicate");
    info.group = DefaultGroup::kDevice;
    info.displayName = "Duplicate";

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    EXPECT_TRUE(manager->addItemInfoCache(info));

    // Adding same item again should fail
    bool result = manager->addItemInfoCache(info);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarInfoCacheMananger, InsertItemInfoCache_Success)
{
    ItemInfo info1;
    info1.url = QUrl::fromLocalFile("/home/first");
    info1.group = DefaultGroup::kCommon;
    info1.displayName = "First";

    ItemInfo info2;
    info2.url = QUrl::fromLocalFile("/home/second");
    info2.group = DefaultGroup::kCommon;
    info2.displayName = "Second";

    manager->addItemInfoCache(info1);

    // Insert at index 0
    bool result = manager->insertItemInfoCache(0, info2);
    EXPECT_TRUE(result);

    auto cacheList = manager->indexCacheList(DefaultGroup::kCommon);
    EXPECT_EQ(cacheList.size(), 2);
    EXPECT_EQ(cacheList[0].url, info2.url);
    EXPECT_EQ(cacheList[1].url, info1.url);
}

TEST_F(UT_SideBarInfoCacheMananger, InsertItemInfoCache_InvalidIndex)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/test");
    info.group = DefaultGroup::kNetwork;
    info.displayName = "Test";

    // Insert at out-of-range index should append
    bool result = manager->insertItemInfoCache(999, info);
    EXPECT_TRUE(result);
    EXPECT_TRUE(manager->contains(info));
}

TEST_F(UT_SideBarInfoCacheMananger, InsertItemInfoCache_NegativeIndex)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/negative");
    info.group = DefaultGroup::kTag;
    info.displayName = "Negative";

    // Insert at negative index should append
    bool result = manager->insertItemInfoCache(-1, info);
    EXPECT_TRUE(result);
    EXPECT_TRUE(manager->contains(info));
}

TEST_F(UT_SideBarInfoCacheMananger, RemoveItemInfoCache_ByGroupAndUrl_Success)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/remove");
    info.group = DefaultGroup::kOther;
    info.displayName = "Remove";

    manager->addItemInfoCache(info);
    EXPECT_TRUE(manager->contains(info.url));

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = manager->removeItemInfoCache(info.group, info.url);
    EXPECT_TRUE(result);
    EXPECT_FALSE(manager->contains(info.url));
}

TEST_F(UT_SideBarInfoCacheMananger, RemoveItemInfoCache_ByGroupAndUrl_NotFound)
{
    QUrl nonExistentUrl = QUrl::fromLocalFile("/non/existent");

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = manager->removeItemInfoCache(DefaultGroup::kCommon, nonExistentUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarInfoCacheMananger, RemoveItemInfoCache_ByUrl_Success)
{
    ItemInfo info1;
    info1.url = QUrl::fromLocalFile("/home/multi1");
    info1.group = DefaultGroup::kCommon;
    info1.displayName = "Multi1";

    ItemInfo info2;
    info2.url = QUrl::fromLocalFile("/home/multi1");
    info2.group = DefaultGroup::kDevice;
    info2.displayName = "Multi2";

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &url1, const QUrl &url2) -> bool {
        __DBG_STUB_INVOKE__
        return url1.path() == url2.path();
    });

    manager->addItemInfoCache(info1);
    manager->addItemInfoCache(info2);

    // Remove by URL should remove from all groups
    bool result = manager->removeItemInfoCache(info1.url);
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarInfoCacheMananger, UpdateItemInfoCache_ByGroupAndUrl_Success)
{
    ItemInfo oldInfo;
    oldInfo.url = QUrl::fromLocalFile("/home/update");
    oldInfo.group = DefaultGroup::kCommon;
    oldInfo.displayName = "OldName";

    manager->addItemInfoCache(oldInfo);

    ItemInfo newInfo = oldInfo;
    newInfo.displayName = "NewName";

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = manager->updateItemInfoCache(oldInfo.group, oldInfo.url, newInfo);
    EXPECT_TRUE(result);

    ItemInfo retrieved = manager->itemInfo(oldInfo.url);
    EXPECT_EQ(retrieved.displayName, QString("NewName"));
}

TEST_F(UT_SideBarInfoCacheMananger, UpdateItemInfoCache_ByGroupAndUrl_NotFound)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/non/existent");
    info.group = DefaultGroup::kCommon;
    info.displayName = "Test";

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = manager->updateItemInfoCache(info.group, info.url, info);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarInfoCacheMananger, UpdateItemInfoCache_ByUrl_Success)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/updateall");
    info.group = DefaultGroup::kNetwork;
    info.displayName = "OldDisplay";

    manager->addItemInfoCache(info);

    ItemInfo newInfo = info;
    newInfo.displayName = "NewDisplay";

    stub.set_lamda(ADDR(UniversalUtils, urlEquals),
        [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = manager->updateItemInfoCache(info.url, newInfo);
    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarInfoCacheMananger, Contains_ItemInfo)
{
    ItemInfo info;
    info.url = QUrl::fromLocalFile("/home/contains");
    info.group = DefaultGroup::kTag;
    info.displayName = "Contains";

    EXPECT_FALSE(manager->contains(info));

    manager->addItemInfoCache(info);
    EXPECT_TRUE(manager->contains(info));
}

TEST_F(UT_SideBarInfoCacheMananger, Contains_Url)
{
    QUrl url = QUrl::fromLocalFile("/home/urltest");

    ItemInfo info;
    info.url = url;
    info.group = DefaultGroup::kCommon;
    info.displayName = "UrlTest";

    EXPECT_FALSE(manager->contains(url));

    manager->addItemInfoCache(info);
    EXPECT_TRUE(manager->contains(url));
}

TEST_F(UT_SideBarInfoCacheMananger, Groups)
{
    ItemInfo info1;
    info1.url = QUrl::fromLocalFile("/home/group1");
    info1.group = DefaultGroup::kCommon;
    info1.displayName = "Group1";

    ItemInfo info2;
    info2.url = QUrl::fromLocalFile("/home/group2");
    info2.group = DefaultGroup::kDevice;
    info2.displayName = "Group2";

    manager->addItemInfoCache(info1);
    manager->addItemInfoCache(info2);

    QStringList groups = manager->groups();
    EXPECT_TRUE(groups.contains(DefaultGroup::kCommon));
    EXPECT_TRUE(groups.contains(DefaultGroup::kDevice));
}

TEST_F(UT_SideBarInfoCacheMananger, IndexCacheList)
{
    ItemInfo info1;
    info1.url = QUrl::fromLocalFile("/home/cache1");
    info1.group = DefaultGroup::kCommon;
    info1.displayName = "Cache1";

    ItemInfo info2;
    info2.url = QUrl::fromLocalFile("/home/cache2");
    info2.group = DefaultGroup::kCommon;
    info2.displayName = "Cache2";

    manager->addItemInfoCache(info1);
    manager->addItemInfoCache(info2);

    auto cacheList = manager->indexCacheList(DefaultGroup::kCommon);
    EXPECT_GE(cacheList.size(), 2);
}

TEST_F(UT_SideBarInfoCacheMananger, ItemInfo)
{
    QUrl url = QUrl::fromLocalFile("/home/iteminfo");

    ItemInfo info;
    info.url = url;
    info.group = DefaultGroup::kOther;
    info.displayName = "ItemInfoTest";

    manager->addItemInfoCache(info);

    ItemInfo retrieved = manager->itemInfo(url);
    EXPECT_EQ(retrieved.url, info.url);
    EXPECT_EQ(retrieved.displayName, info.displayName);
}

TEST_F(UT_SideBarInfoCacheMananger, GetLastSettingKeys)
{
    QStringList keys = manager->getLastSettingKeys();
    EXPECT_TRUE(keys.isEmpty());
}

TEST_F(UT_SideBarInfoCacheMananger, AppendLastSettingKey)
{
    QString key1 = "setting.key1";
    QString key2 = "setting.key2";

    manager->appendLastSettingKey(key1);
    manager->appendLastSettingKey(key2);

    QStringList keys = manager->getLastSettingKeys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(keys.contains(key1));
    EXPECT_TRUE(keys.contains(key2));
}

TEST_F(UT_SideBarInfoCacheMananger, AppendLastSettingKey_Duplicate)
{
    QString key = "duplicate.key";

    manager->appendLastSettingKey(key);
    manager->appendLastSettingKey(key);

    QStringList keys = manager->getLastSettingKeys();
    EXPECT_EQ(keys.size(), 1);
}

TEST_F(UT_SideBarInfoCacheMananger, ClearLastSettingKey)
{
    manager->appendLastSettingKey("key1");
    manager->appendLastSettingKey("key2");

    EXPECT_FALSE(manager->getLastSettingKeys().isEmpty());

    manager->clearLastSettingKey();
    EXPECT_TRUE(manager->getLastSettingKeys().isEmpty());
}

TEST_F(UT_SideBarInfoCacheMananger, GetLastSettingBindingKeys)
{
    QStringList keys = manager->getLastSettingBindingKeys();
    EXPECT_TRUE(keys.isEmpty());
}

TEST_F(UT_SideBarInfoCacheMananger, AppendLastSettingBindingKey)
{
    QString key1 = "binding.key1";
    QString key2 = "binding.key2";

    manager->appendLastSettingBindingKey(key1);
    manager->appendLastSettingBindingKey(key2);

    QStringList keys = manager->getLastSettingBindingKeys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(keys.contains(key1));
    EXPECT_TRUE(keys.contains(key2));
}

TEST_F(UT_SideBarInfoCacheMananger, AppendLastSettingBindingKey_Duplicate)
{
    QString key = "duplicate.binding";

    manager->appendLastSettingBindingKey(key);
    manager->appendLastSettingBindingKey(key);

    QStringList keys = manager->getLastSettingBindingKeys();
    EXPECT_EQ(keys.size(), 1);
}

TEST_F(UT_SideBarInfoCacheMananger, ClearLastSettingBindingKey)
{
    manager->appendLastSettingBindingKey("binding1");
    manager->appendLastSettingBindingKey("binding2");

    EXPECT_FALSE(manager->getLastSettingBindingKeys().isEmpty());

    manager->clearLastSettingBindingKey();
    EXPECT_TRUE(manager->getLastSettingBindingKeys().isEmpty());
}
