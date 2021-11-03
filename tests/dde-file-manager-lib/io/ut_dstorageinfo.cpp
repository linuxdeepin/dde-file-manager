/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             liyigang<liyigang@uniontech.com>
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

#include <gtest/gtest.h>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QRegularExpressionMatch>
#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

#include "dstorageinfo.h"
#include "testhelper.h"
#include "stubext.h"
#include "controllers/vaultcontroller.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE

class DStorageInfoTest:public testing::Test{

public:
    DUrl url;
    StubExt stl;
    virtual void SetUp() override{
        url.setScheme(FILE_SCHEME);
        url.setPath("/");
        std::cout << "start DStorageInfoTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DStorageInfoTest" << std::endl;
    }
};

TEST_F(DStorageInfoTest,can_setPath) {

    DStorageInfo teststorage;
    DStorageInfo teststoragetemp(QDir("~/Pictures/"));

    url.setScheme(FILE_SCHEME);
    url.setPath("~/test_DStrogInfo_test.log");
    QProcess::execute("touch "+url.toLocalFile());
    QProcess::execute("ln -s "+url.toLocalFile() + " " + url.toLocalFile() + "_syslink");
    teststorage.setPath(url.toLocalFile() + "_syslink");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << url.toLocalFile() + "_syslink");
    teststorage.setPath("~/test.log");
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
    stl.set_lamda(&QStorageInfo::bytesTotal,[](){return -1;});
    stl.set_lamda(g_file_find_enclosing_mount,[](){return nullptr;});
    stl.set_lamda(g_mount_get_root,[](){return nullptr;});
    stl.set_lamda(g_file_get_path,[](){
        char *path = new char[16];
        path[0] = '1';
        return path;
    });
    stl.set_lamda(&QStorageInfo::device,[](){return QByteArrayLiteral("gvfsd-fuse");});
    stl.set_lamda(g_file_get_uri,[](){
        char *path = new char[16];
        path[0] = '2';
        return path;
    });
    // stl.set_lamda(g_free,[](){});
    // stl.set_lamda(g_object_unref,[](){});
    QProcess::execute("touch "+url.toLocalFile());
    DStorageInfo teststorage2(url.toLocalFile());
    EXPECT_FALSE(teststorage2.rootPath().isEmpty());
    EXPECT_FALSE(teststorage2.device().isEmpty());
    EXPECT_FALSE(teststorage2.bytesFree() == 0);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_rootPath) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.rootPath().isEmpty());
    storageinfo.setPath("~/test.log");
    storageinfo.refresh();
    storageinfodir.refresh();
    EXPECT_NO_FATAL_FAILURE(storageinfo.rootPath().isEmpty());
    EXPECT_NO_FATAL_FAILURE(storageinfodir.rootPath().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_device) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.device().isEmpty());
    storageinfo.setPath("~/test.log");
    storageinfo.refresh();
    EXPECT_TRUE(storageinfo.device().isEmpty());
    EXPECT_TRUE(storageinfodir.device().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_fileSystemType) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.fileSystemType().isEmpty());
    storageinfo.setPath("~/test.log");
    storageinfo.refresh();
    EXPECT_FALSE(storageinfo.fileSystemType().isEmpty());
    storageinfodir.refresh();
    EXPECT_FALSE(storageinfodir.fileSystemType().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesTotal) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.bytesTotal() != 0);
    storageinfo.setPath("~/test.log");
    storageinfo.refresh();
    EXPECT_TRUE(storageinfo.bytesTotal() != 0);
    EXPECT_FALSE(storageinfodir.bytesTotal() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesFree) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.bytesFree() != 0);
    storageinfo.setPath("~/test.log");
    EXPECT_TRUE(storageinfo.bytesFree() != 0);
    EXPECT_FALSE(storageinfodir.bytesFree() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesAvailable) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    EXPECT_TRUE(storageinfo.bytesAvailable() != 0);
    storageinfo.setPath("~/test.log");
    EXPECT_TRUE(storageinfo.bytesFree() != 0);
    EXPECT_FALSE(storageinfodir.bytesAvailable() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_isReadOnly) {
    DStorageInfo storageinfo;
    EXPECT_FALSE(storageinfo.isReadOnly());
    storageinfo.setPath("/sys/bin");
    storageinfo.refresh();
    EXPECT_FALSE(storageinfo.isReadOnly());
    storageinfo.setPath("/sys/bin/aconnect");
    storageinfo.refresh();
    EXPECT_FALSE(storageinfo.isReadOnly());
}

TEST_F(DStorageInfoTest,can_isLocalDevice) {
    DStorageInfo storageinfo;
    EXPECT_FALSE(storageinfo.isLocalDevice());
    storageinfo.setPath("/sys/bin");
    storageinfo.refresh();
    EXPECT_FALSE(storageinfo.isLocalDevice());
}

TEST_F(DStorageInfoTest,can_isLowSpeedDevice) {
    DStorageInfo storageinfo;
    EXPECT_FALSE(storageinfo.isLowSpeedDevice());
    storageinfo.setPath("/sys/bin");
    EXPECT_FALSE(storageinfo.isLowSpeedDevice());
}

TEST_F(DStorageInfoTest,can_isValid) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfopath("~/test.log");
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    DStorageInfo storageinfoother("~/test.log");
    EXPECT_FALSE(storageinfo.isValid());
    storageinfo.setPath("/sys/bin");
    storageinfo.refresh();
    EXPECT_FALSE(storageinfo.isValid());
}

TEST_F(DStorageInfoTest,can_operator) {
    DStorageInfo storageinfo;
    DStorageInfo storageinfopath("~/test.log");
    DStorageInfo storageinfodir(QDir("~/Pictures/"));
    DStorageInfo storageinfoother("~/test.log");
    EXPECT_FALSE(storageinfo.isValid());
    storageinfo.setPath("/sys/bin");
    DStorageInfo tmp(storageinfo);
    DStorageInfo tmp1(storageinfopath);
    QList<DStorageInfo> infolist;
    infolist << tmp << tmp1;
    qDebug() << tmp;
    qDebug() << tmp1;
    EXPECT_TRUE(infolist.contains(tmp));
    EXPECT_TRUE(infolist.contains(tmp1));
    EXPECT_TRUE(infolist.contains(storageinfodir));
    stl.set_lamda(&DStorageInfo::isValid,[](){return true;});
    stl.set_lamda(&QByteArray::isEmpty,[](){return false;});
    stl.set_lamda(&QString::isEmpty,[](){return false;});
    qDebug() << tmp1;
}

TEST_F(DStorageInfoTest,can_isSameFile) {
    DUrl url1,url2;
    url1.setScheme(FILE_SCHEME);
    url2.setScheme(FILE_SCHEME);
    url1.setPath("~/test.log");
    url2.setPath("~/test.log");
    EXPECT_FALSE(DStorageInfo::isSameFile(url1.path(), url2.path()));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aconnec"));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aclocal"));
}

TEST_F(DStorageInfoTest,can_inSameDevice) {
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnect"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnec"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aclocal"));
    QByteArray(*fileSystemTypelamda)(void *) = [](void *){
        return QByteArray("avfsd");
    };
    stl.set(ADDR(DStorageInfo,fileSystemType),fileSystemTypelamda);
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aclocal"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnect"));
    QByteArray(*fileSystemTypelamda1)(void *) = [](void *){
        return QByteArray("gvfsd-fuse");
    };
    stl.set(ADDR(DStorageInfo,fileSystemType),fileSystemTypelamda1);
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aclocal"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnect"));
}

TEST_F(DStorageInfoTest,can_inSameDevice_url) {
    DUrl url1,url2;
    url1.setScheme(FILE_SCHEME);
    url2.setScheme(FILE_SCHEME);
    url1.setPath("/sys/bin/aconnect");
    url2.setPath("/sys/bin/aconnect");
    EXPECT_TRUE(DStorageInfo::inSameDevice(url1, url2));
    url1.setPath("/sys/bin/aconnect");
    url2.setPath("/sys/bin/aconnec");
    EXPECT_TRUE(DStorageInfo::inSameDevice(url1, url2));
    url1.setPath("/sys/bin/aconnect");
    url2.setPath("/sys/bin/aclocal");
    EXPECT_TRUE(DStorageInfo::inSameDevice(url1, url2));
    DUrl url3,url4,url5;
    url3.setScheme(FILE_SCHEME);
    url4.setScheme(FILE_ROOT);
    url5.setScheme(FILE_SCHEME);
    EXPECT_FALSE(DStorageInfo::inSameDevice(url3, url4));
//    bool (*isLocalFilelamda)(void *) = [](void *){return false;};
//    stl.set(ADDR(DUrl,isLocalFile),isLocalFilelamda);
    EXPECT_TRUE(DStorageInfo::inSameDevice(url3, url5));
}

TEST_F(DStorageInfoTest,can_static_isLocalDevice) {
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
    EXPECT_TRUE(DStorageInfo::isLocalDevice("~/dstorng_test",true));
    stl.set_lamda(&VaultController::isVaultFile,[](){return true;});
    EXPECT_TRUE(DStorageInfo::isLocalDevice("/run/user/1000"));
    stl.set_lamda(&QRegularExpressionMatch::hasMatch,[](){return true;});
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
}

TEST_F(DStorageInfoTest,can_static_isLowSpeedDevice) {
    EXPECT_FALSE(DStorageInfo::isLowSpeedDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
    stl.set_lamda(&QRegularExpressionMatch::hasMatch,[](){return true;});
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
}

TEST_F(DStorageInfoTest,can_static_isCdRomDevice) {
    EXPECT_FALSE(DStorageInfo::isCdRomDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isCdRomDevice("/run/user/1000"));
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}
