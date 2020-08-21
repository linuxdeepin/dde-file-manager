#include <gtest/gtest.h>
#include <QDateTime>
#include <QDebug>

#include "dstorageinfo.h"


using namespace testing;
DFM_USE_NAMESPACE

class DStorageInfoTest:public testing::Test{

public:

    DStorageInfo *storageinfo = nullptr;
    DStorageInfo *storageinfopath = nullptr;
    DStorageInfo *storageinfodir = nullptr;
    DStorageInfo *storageinfoother = nullptr;
    DUrl url;
    virtual void SetUp() override{
        storageinfo = new DStorageInfo();
        storageinfopath = new DStorageInfo("~/test.log");
        storageinfodir = new DStorageInfo(QDir("~/Pictures/"));
        storageinfoother = new DStorageInfo(*storageinfopath);
        url.setScheme(FILE_SCHEME);
        url.setPath("/");
        std::cout << "start DStorageInfoTest" << std::endl;
    }

    virtual void TearDown() override{
        if (storageinfo) {
            delete storageinfo;
            storageinfo = nullptr;
        }
        if (storageinfodir) {
            delete storageinfodir;
            storageinfodir = nullptr;
        }
        if (storageinfopath) {
            delete storageinfopath;
            storageinfopath = nullptr;
        }
        if (storageinfoother) {
            delete storageinfoother;
            storageinfoother = nullptr;
        }

        std::cout << "end DStorageInfoTest" << std::endl;
    }
};

TEST_F(DStorageInfoTest,can_setPath) {
    DStorageInfo teststorage = *storageinfo;
    teststorage.setPath("~/test.log");
}

TEST_F(DStorageInfoTest,can_rootPath) {
    EXPECT_TRUE(storageinfo->rootPath().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    storageinfodir->refresh();
    EXPECT_TRUE(storageinfo->rootPath().isEmpty());
    EXPECT_TRUE(storageinfodir->rootPath().isEmpty());
}

TEST_F(DStorageInfoTest,can_device) {
    EXPECT_TRUE(storageinfo->device().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_TRUE(storageinfo->device().isEmpty());
    EXPECT_TRUE(storageinfodir->device().isEmpty());
}

TEST_F(DStorageInfoTest,can_fileSystemType) {
    EXPECT_TRUE(storageinfo->fileSystemType().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->fileSystemType().isEmpty());
    storageinfodir->refresh();
    EXPECT_FALSE(storageinfodir->fileSystemType().isEmpty());
}

TEST_F(DStorageInfoTest,can_bytesTotal) {
    EXPECT_TRUE(storageinfo->bytesTotal() != 0);
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_TRUE(storageinfo->bytesTotal() != 0);
    EXPECT_FALSE(storageinfodir->bytesTotal() == 0);
}

TEST_F(DStorageInfoTest,can_bytesFree) {
    EXPECT_TRUE(storageinfo->bytesFree() != 0);
    storageinfo->setPath("~/test.log");
    EXPECT_TRUE(storageinfo->bytesFree() != 0);
    EXPECT_FALSE(storageinfodir->bytesFree() == 0);
}

TEST_F(DStorageInfoTest,can_isReadOnly) {
    EXPECT_FALSE(storageinfo->isReadOnly());
    storageinfo->setPath("/sys/bin");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->isReadOnly());
    storageinfo->setPath("/sys/bin/aconnect");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->isReadOnly());
}

TEST_F(DStorageInfoTest,can_isLocalDevice) {
    EXPECT_FALSE(storageinfo->isLocalDevice());
    storageinfo->setPath("/sys/bin");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->isLocalDevice());
}

TEST_F(DStorageInfoTest,can_isLowSpeedDevice) {
    EXPECT_FALSE(storageinfo->isLowSpeedDevice());
    storageinfo->setPath("/sys/bin");
    EXPECT_FALSE(storageinfo->isLowSpeedDevice());
}

TEST_F(DStorageInfoTest,can_isValid) {
    EXPECT_FALSE(storageinfo->isValid());
    storageinfo->setPath("/sys/bin");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->isValid());
}

TEST_F(DStorageInfoTest,can_operator) {
    EXPECT_FALSE(storageinfo->isValid());
    storageinfo->setPath("/sys/bin");
    DStorageInfo tmp = *storageinfo;
    DStorageInfo tmp1 = *storageinfopath;
    QList<DStorageInfo> infolist;
    infolist << tmp << tmp1;
    qDebug() << tmp;
    EXPECT_TRUE(infolist.contains(tmp));
    EXPECT_TRUE(infolist.contains(tmp1));
    EXPECT_TRUE(infolist.contains(*storageinfodir));
}

TEST_F(DStorageInfoTest,can_isSameFile) {
    EXPECT_TRUE(DStorageInfo::isSameFile("~/sys", "~/sys"));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aconnec"));
    EXPECT_FALSE(DStorageInfo::isSameFile("/sys/bin/aconnect", "/sys/bin/aclocal"));
}

TEST_F(DStorageInfoTest,can_inSameDevice) {
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnect"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnec"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aclocal"));
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
}

TEST_F(DStorageInfoTest,can_static_isLocalDevice) {
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
}

TEST_F(DStorageInfoTest,can_static_isLowSpeedDevice) {
    EXPECT_FALSE(DStorageInfo::isLowSpeedDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isLocalDevice("/run/user/1000"));
}

TEST_F(DStorageInfoTest,can_static_isCdRomDevice) {
    EXPECT_FALSE(DStorageInfo::isCdRomDevice("/sys/bin/aconnect"));
    EXPECT_FALSE(DStorageInfo::isCdRomDevice("/run/user/1000"));
}
