/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include <gio/gio.h>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "dfmevent.h"
#include "gvfs/networkmanager.h"
#include "utils/singleton.h"
#include "app/define.h"
#include "stubext.h"
#include "views/windowmanager.h"
#include <QWidget>
#include "dfmevent.h"


namespace {
class TestNetworkManager: public testing::Test
{
public:
    NetworkManager *m_manager = Singleton<NetworkManager>::instance();
    stub_ext::StubExt stubx;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    QWidget tmpWidget;
};
}


GFileInfo *new_g_file_info (void)
{
  GFileInfo *info = g_file_info_new ();

#define TEST_NAME			"Prilis zlutoucky kun"
#define TEST_DISPLAY_NAME	        "UTF-8 p\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88"
#define TEST_SIZE			0xFFFFFFF0

  g_file_info_set_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME, TEST_NAME);
  g_file_info_set_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, TEST_DISPLAY_NAME);
  g_file_info_set_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE, TEST_SIZE);
  g_file_info_set_file_type (info, G_FILE_TYPE_DIRECTORY);

  //g_object_unref (info);
   return info;
}


#include <deviceinfo/udiskdeviceinfo.h>
#include <deviceinfo/udisklistener.h>
TEST_F(TestNetworkManager, fetchNetworks)
{
    QEventLoop event_loop;

    stubx.set_lamda(&WindowManager::getWindowById, [this]{
        return &tmpWidget;
    });

    stubx.set_lamda(&QEventLoop::exec, []{
        return 0;
    });

    stubx.set_lamda(g_file_enumerate_children_async, [](){});

    Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(nullptr, DUrl("smb:///")));

    stubx.set_lamda(&UDiskListener::getDeviceByMountPoint, []{
        UDiskDeviceInfo* device = new UDiskDeviceInfo();
        return UDiskDeviceInfoPointer(device);
    });
    stubx.set_lamda(&UDiskDeviceInfo::getMountPointUrl, []{
        return DUrl("smb:///");
    });
    Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(nullptr, DUrl("smb:///")));
    stubx.reset(&UDiskListener::getDeviceByMountPoint);
    stubx.reset(&UDiskDeviceInfo::getMountPointUrl);


    stubx.set_lamda(&UDiskListener::getDeviceByMountPoint, []{
        UDiskDeviceInfo* device = new UDiskDeviceInfo();
        return UDiskDeviceInfoPointer(device);
    });
    stubx.set_lamda(&UDiskDeviceInfo::getMountPointUrl, []{
        return DUrl("notsmb:///");
    });
    Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(nullptr, DUrl("smb:///")));
    stubx.reset(&UDiskListener::getDeviceByMountPoint);
    stubx.reset(&UDiskDeviceInfo::getMountPointUrl);

    GObject *source_object = nullptr;
    GAsyncResult *res = nullptr;
    gpointer user_data = nullptr;
    Singleton<NetworkManager>::instance()->network_enumeration_finished(source_object, res, user_data);

    GFileEnumerator *enumerator = nullptr;
    GFileInfo *info = new_g_file_info();
    GList *detected_networks = g_list_alloc();
    detected_networks->data = info;
    DFMUrlBaseEvent *event = new DFMUrlBaseEvent(nullptr, DUrl("smb:///"));
    Singleton<NetworkManager>::instance()->populate_networks(enumerator, detected_networks, gpointer(event));
    Singleton<NetworkManager>::instance()->eventLoop = &event_loop;
    Singleton<NetworkManager>::instance()->cancelFeatchNetworks();
    delete event;
    event = nullptr;
}


namespace {
class TestNetworkNode: public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};
}

TEST_F(TestNetworkNode, url)
{
    NetworkNode node;
    QString urlname = "urlname";
    QString displayname = "displayname";
    QString icontype = "icontype";

    node.setUrl(urlname);
    node.setDisplayName(displayname);
    node.setIconType(icontype);

    EXPECT_STREQ(node.url().toStdString().c_str(), urlname.toStdString().c_str());
    EXPECT_STREQ(node.displayName().toStdString().c_str(), displayname.toStdString().c_str());
    EXPECT_STREQ(node.iconType().toStdString().c_str(), icontype.toStdString().c_str());

    qDebug() << node;
}
