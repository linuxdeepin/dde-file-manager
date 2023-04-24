// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FILEUTILS
#define UT_FILEUTILS
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <dfm-io/dfmio_utils.h>

#include <gtest/gtest.h>
#include "stubext.h"

DFMBASE_USE_NAMESPACE

class UT_FileUtils : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

     stub_ext::StubExt stub;
};

TEST_F(UT_FileUtils, testIsContainProhibitPath)
{
    QList<QUrl> pathUrls;
    pathUrls << QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());

    EXPECT_EQ(true, FileUtils::isContainProhibitPath(pathUrls));

    QList<QUrl> tempUrls;
    tempUrls << QUrl("/usr/test");
    EXPECT_EQ(false, FileUtils::isContainProhibitPath(tempUrls));

    tempUrls << QUrl("/usr/test")
             << QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    EXPECT_EQ(true, FileUtils::isContainProhibitPath(tempUrls));
}

TEST_F(UT_FileUtils, bug_186753_fileInDlnfsIsLocalFile)
{
   stub.set_lamda(&QUrl::isLocalFile, [] { return false; }); // Qt判断非本地文件
   stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [] { return true; }); // 判断为dlnfs 挂载
   stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, [] { return false; }); // 非外部块设备挂载

   QUrl url("doc.dlnfs.mount.file");
   EXPECT_TRUE(FileUtils::isLocalFile(url));
}

TEST_F(UT_FileUtils, bug_186753_fileInDlnfsIsNotLocalFile)
{
   stub.set_lamda(&QUrl::isLocalFile, [] { return false; }); // Qt判断非本地文件
   stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [] { return true; }); // 判断为dlnfs 挂载
   stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, [] { return true; }); // 外部块设备挂载

   QUrl url("ext4.dlnfs.mount.file");
   EXPECT_FALSE(FileUtils::isLocalFile(url));
}

TEST_F(UT_FileUtils, bug_186753_fileNotInDlnfsIsNotLocalFile)
{
   stub.set_lamda(&QUrl::isLocalFile, [] { return false; }); // Qt判断非本地文件
   stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [] { return false; }); // 判断为非dlnfs 挂载

   QUrl url("ext4.dlnfs.mount.file");
   EXPECT_FALSE(FileUtils::isLocalFile(url));
}

TEST_F(UT_FileUtils, bug_186753_dlnfsDeviceIsLocalDevice)
{
   stub.set_lamda(&FileUtils::isGvfsFile, [] { return false; }); // 判断非gvfs 挂载
   stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [] { return true; }); // dfm-io 第一层判断文件属于移动设备
   stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [] { return true; }); // 判断为dlnfs 挂载
   stub.set_lamda(&DeviceProxyManager::isFileOfExternalBlockMounts, [] { return false; }); // 非外部块设备挂载

   QUrl url("doc.dlnfs.mount.file");
   EXPECT_TRUE(FileUtils::isLocalDevice(url));
}

TEST_F(UT_FileUtils, bug_186753_dlnfsDeviceIsLocalDevice2)
{
   stub.set_lamda(&FileUtils::isGvfsFile, [] { return false; }); // 判断非gvfs 挂载
   stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [] { return false; }); // dfm-io 第一层判断文件属于非移动设备

   QUrl url("dev.local.mount.file");
   EXPECT_TRUE(FileUtils::isLocalDevice(url));
}

TEST_F(UT_FileUtils, bug_186753_notdlnfsDeviceIsNotLocalDevice)
{
    stub.set_lamda(&FileUtils::isGvfsFile, [] { return false; }); // 判断非gvfs 挂载
    stub.set_lamda(&DFMIO::DFMUtils::fileIsRemovable, [] { return true; }); // dfm-io 第一层判断文件属于移动设备
    stub.set_lamda(&DeviceUtils::isSubpathOfDlnfs, [] { return false; }); // 判断为dlnfs 挂载

   QUrl url("external.dev.mount.file");
   EXPECT_FALSE(FileUtils::isLocalDevice(url));
}

#endif
