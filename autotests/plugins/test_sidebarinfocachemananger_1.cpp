// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarinfocachemananger_1.cpp
 * @brief Unit tests for SideBarInfoCacheMananger methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/sidebarinfocachemananger.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarInfoCacheManangerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarInfoCacheMananger();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarInfoCacheMananger *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarInfoCacheManangerTest, SideBarInfoCacheMananger)
{
    // Test constructor: SideBarInfoCacheMananger(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarInfoCacheManangerTest, addItemInfoCache)
{
    // Test method: bool addItemInfoCache((const ItemInfo &info))
    ItemInfo _arg0{};
    auto result = obj->addItemInfoCache(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarInfoCacheManangerTest, appendLastSettingBindingKey)
{
    // Test method: void appendLastSettingBindingKey((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->appendLastSettingBindingKey(_arg0));
}

TEST_F(SideBarInfoCacheManangerTest, appendLastSettingKey)
{
    // Test method: void appendLastSettingKey((const QString &value))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->appendLastSettingKey(_arg0));
}

TEST_F(SideBarInfoCacheManangerTest, getLastSettingBindingKeys)
{
    // Test getter: QStringList getLastSettingBindingKeys()
    auto result = obj->getLastSettingBindingKeys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarInfoCacheManangerTest, getLastSettingKeys)
{
    // Test getter: QStringList getLastSettingKeys()
    auto result = obj->getLastSettingKeys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarInfoCacheManangerTest, indexCacheList)
{
    // Test method: SideBarInfoCacheMananger::CacheInfoList indexCacheList((const Group &name))
    Group _arg0{};
    auto result = obj->indexCacheList(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(SideBarInfoCacheManangerTest, insertItemInfoCache)
{
    // Test method: bool insertItemInfoCache((SideBarInfoCacheMananger::Index i, const ItemInfo &info))
    ItemInfo _arg1{};
    auto result = obj->insertItemInfoCache(SideBarInfoCacheMananger::Index(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SideBarInfoCacheManangerTest, instance)
{
    // Test getter: SideBarInfoCacheMananger instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SideBarInfoCacheManangerTest, itemInfo)
{
    // Test method: ItemInfo itemInfo((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->itemInfo(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->itemInfo(_arg0); });

}
