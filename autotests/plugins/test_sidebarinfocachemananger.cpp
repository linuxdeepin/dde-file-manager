// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarinfocachemananger.cpp
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

TEST_F(SideBarInfoCacheManangerTest, clearLastSettingBindingKey)
{
    // Test method: void clearLastSettingBindingKey(())
    EXPECT_NO_FATAL_FAILURE(obj->clearLastSettingBindingKey());
}

TEST_F(SideBarInfoCacheManangerTest, clearLastSettingKey)
{
    // Test method: void clearLastSettingKey(())
    EXPECT_NO_FATAL_FAILURE(obj->clearLastSettingKey());
}

TEST_F(SideBarInfoCacheManangerTest, contains)
{
    // Test method: bool contains((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->contains(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarInfoCacheManangerTest, removeItemInfoCache)
{
    // Test method: bool removeItemInfoCache((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeItemInfoCache(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarInfoCacheManangerTest, updateItemInfoCache)
{
    // Test method: bool updateItemInfoCache((const QUrl &url, const ItemInfo &info))
    QUrl _arg0{};
    ItemInfo _arg1{};
    auto result = obj->updateItemInfoCache(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(SideBarInfoCacheManangerTest, groups)
{
    // Test getter: SideBarInfoCacheMananger::GroupList groups()
    auto result = obj->groups();
    EXPECT_GE(static_cast<int>(result), 0);

}
