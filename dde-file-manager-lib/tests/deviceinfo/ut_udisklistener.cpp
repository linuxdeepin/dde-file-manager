/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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
#include "deviceinfo/udisklistener.h"
#include "controllers/subscriber.h"
#include "dfmevent.h"

#include "stub.h"
#include "addr_pri.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

ACCESS_PRIVATE_FUN(UDiskListener, void(const QString &labelName), fileSystemDeviceIdLabelChanged);

namespace {
class TestUDiskListener: public testing::Test {
public:
    QDiskInfo m_diskInfo;
    UDiskListener *m_listener {nullptr};
    UDiskDeviceInfoPointer m_devInfo {nullptr};

    void SetUp() override
    {
        m_listener = new UDiskListener;
        m_devInfo = new UDiskDeviceInfo;
        m_diskInfo.setId("123");
        m_diskInfo.setName("abc");
        m_diskInfo.setType("dvd");
        m_diskInfo.setUnix_device("/deb/sdb");
        m_diskInfo.setUuid("abcd-efgh");
        m_diskInfo.setMounted_root_uri("/media");
        m_diskInfo.setIconName(":computer");
        m_diskInfo.setCan_unmount(true);
        m_diskInfo.setCan_eject(true);
        m_diskInfo.setUsed(true);
        m_diskInfo.setTotal(65536);
        m_diskInfo.setFree(1024);
        m_diskInfo.setIsNativeCustom(true);
        m_diskInfo.setCan_mount(true);
        m_diskInfo.setRead_only(true);
        m_diskInfo.setActivation_root_uri("/");
        m_diskInfo.setIs_removable(true);
        m_diskInfo.setHas_volume(true);
        m_diskInfo.setId_filesystem("ntfs");
        m_diskInfo.setDefault_location("/");
        m_diskInfo.setDrive_unix_device("0");
        m_devInfo->setDiskInfo(m_diskInfo);
        m_listener->addDevice(m_devInfo);
    }
    void TearDown() override
    {
        delete m_listener;
    }
};

class TestSubscriber: public Subscriber {
public:
    void doSubscriberAction(const QString& path) override {
        qDebug() << path;
    }
};

}

TEST_F(TestUDiskListener, device)
{
    UDiskDeviceInfoPointer device = m_listener->getDevice("123");
    EXPECT_STREQ(device->getId().toStdString().c_str(), m_diskInfo.id().toStdString().c_str());
    EXPECT_TRUE(m_listener->hasDeviceInfo("123"));
    EXPECT_EQ(m_listener->getAllDeviceInfos().size(), 1);
    EXPECT_EQ(m_listener->getDeviceList().size(), 1);
    m_listener->removeDevice(device);
    EXPECT_FALSE(m_listener->getDevice("123"));
    m_listener->addDevice(m_devInfo);
}

TEST_F(TestUDiskListener, renameFile)
{
    // create test file
    QString path1(QDir::homePath() + "/udisklistener_test1.txt");
    if (!QFile::exists(path1)) {
        QFile file1(path1);
        file1.open(QIODevice::WriteOnly | QIODevice::Text);
        file1.close();

        QString path2(QDir::homePath() + "~/udisklistener_test2.txt");

        if (!QFile::exists(path2)) {
            DUrl from = DUrl::fromLocalFile(path1);
            DUrl to = DUrl::fromLocalFile(path2);
            QSharedPointer<DFMRenameEvent> event(new DFMRenameEvent(nullptr, from, to));

            EXPECT_TRUE(m_listener->renameFile(event));
            QFile::remove(path2);
        }
        QFile::remove(path1);
    }
}

TEST_F(TestUDiskListener, lastPart)
{
    EXPECT_STREQ(m_listener->lastPart("/ab/cd/ef/gh").toStdString().c_str(), "gh");
}

TEST_F(TestUDiskListener, subscriber)
{
    TestSubscriber subriber;
    EXPECT_NO_FATAL_FAILURE(m_listener->addSubscriber(&subriber));
    EXPECT_NO_FATAL_FAILURE(m_listener->removeSubscriber(&subriber));
}

TEST_F(TestUDiskListener, isDeviceFolder)
{
   EXPECT_FALSE(m_listener->isDeviceFolder("/media"));
}

TEST_F(TestUDiskListener, isInDeviceFolder)
{
    EXPECT_FALSE(m_listener->isInDeviceFolder("/media"));
}

TEST_F(TestUDiskListener, isInRemovableDeviceFolder)
{
    EXPECT_FALSE(m_listener->isInRemovableDeviceFolder("/media"));
}

TEST_F(TestUDiskListener, getChildren)
{
   EXPECT_FALSE(m_listener->getChildren(dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl(),
                                                                                QStringList(), QDir::AllDirs)).isEmpty());
}

TEST_F(TestUDiskListener, createFileInfo)
{
    EXPECT_FALSE(m_listener->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl())));
    DUrl url;
    url.setPath("/etc/apt/sources.list");
    EXPECT_FALSE(m_listener->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url)));
}

TEST_F(TestUDiskListener, createFileWatcher)
{
    DUrl url;
    url.setPath("/etc/apt/");
    EXPECT_TRUE(m_listener->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url)));
}

TEST_F(TestUDiskListener, getDeviceByDevicePath)
{
    EXPECT_TRUE(m_listener->getDeviceByDevicePath("/deb/sdb"));
}

TEST_F(TestUDiskListener, getDeviceByMountPoint)
{
    EXPECT_FALSE(m_listener->getDeviceByMountPoint("/"));
    EXPECT_FALSE(m_listener->getDeviceByMountPoint("smb://127.0.0.1/"));

}

TEST_F(TestUDiskListener, getDeviceByMountPointFilePath)
{
    EXPECT_TRUE(m_listener->getDeviceByMountPointFilePath("/"));
}

TEST_F(TestUDiskListener, getDeviceByPath)
{
    EXPECT_FALSE(m_listener->getDeviceByPath("/"));
}

TEST_F(TestUDiskListener, getDeviceByFilePath)
{
    EXPECT_TRUE(m_listener->getDeviceByFilePath("/"));
}

TEST_F(TestUDiskListener, getDeviceByDeviceID)
{
    EXPECT_TRUE(m_listener->getDeviceByDeviceID("123"));
}

TEST_F(TestUDiskListener, update)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->update());
}

TEST_F(TestUDiskListener, diskInfos)
{
    EXPECT_EQ(m_listener->getMountList().size(), 0);
    QDiskInfo info;
    info.setId("abc");
    info.setMounted_root_uri("/");
    m_listener->addMountDiskInfo(info);
    EXPECT_EQ(m_listener->getMountList().size(), 1);
    m_listener->removeMountDiskInfo(info);
    EXPECT_EQ(m_listener->getMountList().size(), 0);
}

TEST_F(TestUDiskListener, volumeInfos)
{
    QDiskInfo info;
    info.setId("000");
    info.setMounted_root_uri("/");
    EXPECT_FALSE(m_listener->getAllDeviceInfos().contains("000"));
    m_listener->addVolumeDiskInfo(info);
    EXPECT_TRUE(m_listener->getAllDeviceInfos().contains("000"));
    info.setMounted_root_uri("/etc");
    m_listener->changeVolumeDiskInfo(info);
    EXPECT_TRUE(m_listener->getAllDeviceInfos().contains("000"));
    m_listener->removeVolumeDiskInfo(info);
    EXPECT_FALSE(m_listener->getAllDeviceInfos().contains("000"));
}

TEST_F(TestUDiskListener, eject)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->eject("/dev/sr0"));
}

TEST_F(TestUDiskListener, mountByUDisks)
{
    EXPECT_FALSE(m_listener->mountByUDisks(""));
    EXPECT_FALSE(m_listener->mountByUDisks("/dev/sda1"));
}

TEST_F(TestUDiskListener, unmount)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->unmount("/dev/sdb1"));
}

TEST_F(TestUDiskListener, stopDrive)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->stopDrive(""));
}

TEST_F(TestUDiskListener, forceUnmount)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->forceUnmount(""));
}

TEST_F(TestUDiskListener, fileSystemDeviceIdLabelChanged)
{
    EXPECT_NO_FATAL_FAILURE (
        call_private_fun::UDiskListenerfileSystemDeviceIdLabelChanged(*m_listener, "");
    );
}
