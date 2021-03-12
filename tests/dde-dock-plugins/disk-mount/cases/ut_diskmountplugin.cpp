/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_mock_pluginproxyinterface.h"

#include <QWidget>
#include <gtest/gtest.h>

#define protected public

#include "diskmountplugin.h"

namespace  {
    class TestDiskMountPlugin : public testing::Test {
    public:

        void SetUp() override
        {
            mDiskMountPlugin.reset( new DiskMountPlugin(false));
            mDiskMountPlugin->init(&mock_proxy);
        }
        void TearDown() override
        {
        }

    public:
        MockPluginProxyInterface mock_proxy;
        std::shared_ptr<DiskMountPlugin> mDiskMountPlugin;
    };
}

static const QString DISK_MOUNT = "disk-mount";

TEST_F(TestDiskMountPlugin, can_normal_uninite_check)
{
    DiskMountPlugin mountPlugin;
    EXPECT_EQ( DISK_MOUNT, mountPlugin.pluginName() );
    EXPECT_EQ( DISK_MOUNT, mDiskMountPlugin->pluginName() );

    EXPECT_TRUE( nullptr != mountPlugin.itemWidget("dummy") );
    EXPECT_TRUE( nullptr != mountPlugin.itemTipsWidget("dummy") );
    EXPECT_TRUE( nullptr == mountPlugin.itemPopupApplet("dummy") );
}

TEST_F(TestDiskMountPlugin, can_notify_tips)
{
    TipsWidget tipsWidget;
    tipsWidget.refreshFont();
    tipsWidget.paintEvent(nullptr);
}

TEST_F(TestDiskMountPlugin, can_be_init)
{
    EXPECT_EQ( DISK_MOUNT, mDiskMountPlugin->pluginName() );
    EXPECT_TRUE( nullptr != mDiskMountPlugin->itemPopupApplet("dummy") );
}

TEST_F(TestDiskMountPlugin, can_get_menu_list)
{
    QString menulist = mDiskMountPlugin->itemContextMenu("menu");
    EXPECT_TRUE( menulist.contains("Eject all") );
}

TEST_F(TestDiskMountPlugin, can_invoke_menu_open)
{
    QString menulist = mDiskMountPlugin->itemContextMenu("open");
    EXPECT_TRUE( menulist.contains("Eject all") );

    mDiskMountPlugin->invokedMenuItem("itemKey","open",true);
}

TEST_F(TestDiskMountPlugin, can_invoke_menu_unmount_all)
{
    QString menulist = mDiskMountPlugin->itemContextMenu("unmount_all");
    EXPECT_TRUE( menulist.contains("Eject all") );

    ON_CALL(mock_proxy,itemAdded);
    ON_CALL(mock_proxy,itemRemoved);

    mDiskMountPlugin->invokedMenuItem("itemKey","unmount_all",true);
}

TEST_F(TestDiskMountPlugin, can_do_itemSortKey_setSortKey)
{
    EXPECT_CALL(mock_proxy,saveValue).Times(1);

    mDiskMountPlugin->setSortKey("disk-mount-init-value",6);

    EXPECT_CALL(mock_proxy,getValue).WillOnce(testing::Return(6));

    EXPECT_FALSE( mDiskMountPlugin->itemSortKey("disk-mount-init-value") == 5);
}

TEST_F(TestDiskMountPlugin, can_get_default_itemSortKey)
{
    EXPECT_CALL(mock_proxy,getValue).WillOnce(testing::Return(5));
    EXPECT_EQ(5, mDiskMountPlugin->itemSortKey("disk-mount-init-value"));
}

TEST_F(TestDiskMountPlugin, can_refresh_itemIcon)
{
    EXPECT_EQ( DISK_MOUNT, mDiskMountPlugin->pluginName() );
    mDiskMountPlugin->refreshIcon(DISK_MOUNT_KEY);
}
