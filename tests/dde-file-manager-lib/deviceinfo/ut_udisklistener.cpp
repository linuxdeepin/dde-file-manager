/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "controllers/subscriber.h"
#include "dfmevent.h"
#include "singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "interfaces/dfileservices.h"
#include "dabstractfilewatcher.h"

#include "addr_pri.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define private public
#include "deviceinfo/udisklistener.h"

ACCESS_PRIVATE_FUN(UDiskListener, void(const QString &labelName), fileSystemDeviceIdLabelChanged);

using namespace stub_ext;

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
        // 构造 UDiskListener 时，会自动注册，先取消注册，再删除，避免被重复析构
        // 在 UDiskLister 析构函数实现更好
        // 改为在析构的时候自动解注册 2021 05 10 lanxs
        //DFileService::unsetFileUrlHandler(m_listener);
        delete m_listener;
        m_listener = nullptr;
    }
};

class TestSubscriber: public Subscriber {
public:
    void doSubscriberAction(const QString& path) override {
        qDebug() << path;
    }
};

}


TEST_F(TestUDiskListener, signal)
{
    Stub st;
    m_listener->m_nCDRomCount = 1;
    EXPECT_NO_FATAL_FAILURE(fileSignalManager->restartCdScanTimer(""));
    EXPECT_NO_FATAL_FAILURE(fileSignalManager->stopCdScanTimer(""));
    EXPECT_NO_FATAL_FAILURE(m_listener->m_diskMgr->fileSystemRemoved(""));
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

//TEST_F(TestUDiskListener, renameFile)
//{
//    // create test file
//    QString path1(QDir::currentPath() + "/udisklistener_test1.txt");
//    if (!QFile::exists(path1)) {
//        QFile file1(path1);
//        file1.open(QIODevice::WriteOnly | QIODevice::Text);
//        file1.close();

//        QString path2(QDir::currentPath() + "~/udisklistener_test2.txt");

//        if (!QFile::exists(path2)) {
//            DUrl from = DUrl::fromLocalFile(path1);
//            DUrl to = DUrl::fromLocalFile(path2);
//            QSharedPointer<DFMRenameEvent> event(new DFMRenameEvent(nullptr, from, to));

//            EXPECT_TRUE(m_listener->renameFile(event));
//            QFile::remove(path2);
//        }
//        QFile::remove(path1);
//    }
//}

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
    DAbstractFileInfoPointer info = m_listener->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl()));
    EXPECT_FALSE(info);
    DUrl url;
    url.setPath("/etc/apt/sources.list");
    info = m_listener->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_FALSE(info);
}

TEST_F(TestUDiskListener, createFileWatcher)
{
    DUrl url;
    url.setPath("/etc/apt/");
    QScopedPointer<DAbstractFileWatcher> watcher;
    watcher.reset(m_listener->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url)));
    EXPECT_TRUE(watcher != nullptr);
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
    StubExt st;
    st.set_lamda(ADDR(UDiskDeviceInfo, getMountPointUrl), []() {
        return DUrl::fromLocalFile("/media/");
    });

    EXPECT_TRUE(m_listener->getDeviceByFilePath("/media/abc"));
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

/*
TEST_F(TestUDiskListener, eject)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->eject("/dev/sr0"));
}*/

TEST_F(TestUDiskListener, mountByUDisks)
{
    EXPECT_FALSE(m_listener->mountByUDisks(""));

    StubExt stubex;
    stubex.set_lamda(ADDR(DBlockDevice, mount), []() {
        return "/media/sda1";
    });
    EXPECT_NO_FATAL_FAILURE(m_listener->mountByUDisks("/dev/sda1")); // 有些服务器并没有这个节点 所以暂时使用 EXPECT_NO_FATAL_FAILURE
}

/* unmount sdb1 is dangerous
TEST_F(TestUDiskListener, unmount)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->unmount("/dev/sdb1"));
}*/

TEST_F(TestUDiskListener, stopDrive)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->stopDrive(""));
}

/*
TEST_F(TestUDiskListener, forceUnmount)
{
    UDiskListener listener;
    listener.m_map["a"] = new UDiskDeviceInfo;
    EXPECT_NO_FATAL_FAILURE(listener.forceUnmount("a"));
    //DFileService::unsetFileUrlHandler(&listener);
}*/

TEST_F(TestUDiskListener, fileSystemDeviceIdLabelChanged)
{
    EXPECT_NO_FATAL_FAILURE (
        call_private_fun::UDiskListenerfileSystemDeviceIdLabelChanged(*m_listener, "");
    );
}

TEST_F(TestUDiskListener, loopCheckCD)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->loopCheckCD());
}

TEST_F(TestUDiskListener, getVolumeLetters)
{
    EXPECT_EQ(m_listener->getVolumeLetters().size(), 0);
}

TEST_F(TestUDiskListener, getMountedRemovableDiskDeviceInfos)
{
    EXPECT_TRUE(m_listener->getMountedRemovableDiskDeviceInfos().size() > 0);
}

TEST_F(TestUDiskListener, getCanSendDisksByUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->getCanSendDisksByUrl("/"));
}

TEST_F(TestUDiskListener, isMountedRemovableDiskExits)
{
    // 阻塞CI
    // EXPECT_TRUE(m_listener->isMountedRemovableDiskExits());
}


TEST_F(TestUDiskListener, isFileFromDisc)
{
    EXPECT_FALSE(m_listener->isFileFromDisc("/dev/sr0"));
}


TEST_F(TestUDiskListener, appendHiddenDirs)
{
    int n1 = m_listener->hiddenDirs().size();
    m_listener->appendHiddenDirs("/tmp/test");
    int n2 = m_listener->hiddenDirs().size();
    bool ret = ((n2 - n1) == 1);
    EXPECT_TRUE(ret);
}

TEST_F(TestUDiskListener, addMountDiskInfo)
{
    UDiskListener listener;
    QDiskInfo info;
    info.setId("123");
    info.setName("abc");
    info.setType("dvd");
    info.setUnix_device("/deb/sdb");
    info.setUuid("abcd-efgh");
    info.setMounted_root_uri("/media");
    info.setIconName(":computer");
    info.setCan_unmount(true);
    info.setCan_eject(true);
    info.setUsed(true);
    info.setTotal(65536);
    info.setFree(1024);
    info.setIsNativeCustom(true);
    info.setCan_mount(true);
    info.setRead_only(true);
    info.setActivation_root_uri("/");
    info.setIs_removable(true);
    info.setHas_volume(true);
    info.setId_filesystem("ntfs");
    info.setDefault_location("/");
    info.setDrive_unix_device("0");

    TestSubscriber *subscriber = new TestSubscriber;
    listener.m_subscribers.append(subscriber);

    EXPECT_NO_FATAL_FAILURE(listener.addMountDiskInfo(info));

    StubExt st;
    st.set_lamda(&DDiskManager::resolveDeviceNode, []() {
        return  QStringList() << "test";
    });

    EXPECT_NO_FATAL_FAILURE(listener.addMountDiskInfo(info));
    delete subscriber;
    //DFileService::unsetFileUrlHandler(&listener);
}

TEST_F(TestUDiskListener, mount)
{
    EXPECT_NO_FATAL_FAILURE(m_listener->mount("/dev/sdb*"));
}

TEST_F(TestUDiskListener, removeVolumeDiskInfo)
{
    UDiskListener listener;
    QDiskInfo info;
    info.setId("not_exists");
    info.setUuid("test");
    listener.addMountDiskInfo(info);

    UDiskDeviceInfoPointer device;
    device = new UDiskDeviceInfo();
    listener.m_list.append(device);
    listener.m_map.clear();

    EXPECT_NO_FATAL_FAILURE(listener.removeVolumeDiskInfo(info));
    //DFileService::unsetFileUrlHandler(&listener);
}

TEST_F(TestUDiskListener, changeVolumeDiskInfo)
{
    UDiskListener listener;
    QDiskInfo info;

    EXPECT_NO_FATAL_FAILURE(listener.changeVolumeDiskInfo(info));

    info.setId("not_exists");
    listener.addMountDiskInfo(info);
    EXPECT_NO_FATAL_FAILURE(listener.changeVolumeDiskInfo(info));

    listener.m_map.clear();
    EXPECT_NO_FATAL_FAILURE(listener.changeVolumeDiskInfo(info));
    //DFileService::unsetFileUrlHandler(&listener);
}

TEST_F(TestUDiskListener, isBlockFile)
{
    StubExt st;
    st.set_lamda(ADDR(UDiskListener, getMountedRemovableDiskDeviceInfos), []() {
        QMap<QString, UDiskDeviceInfoPointer> infos;
        UDiskDeviceInfoPointer p(new UDiskDeviceInfo);
        infos["test"] = p;
        return infos;
    });

    st.set_lamda(ADDR(UDiskDeviceInfo, getMountPointUrl), []() {
        return DUrl("/test/");
    });
    EXPECT_TRUE(m_listener->isBlockFile("/test/_test_/_test_path_"));
}

