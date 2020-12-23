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

    DStorageInfo *storageinfo = nullptr;
    DStorageInfo *storageinfopath = nullptr;
    DStorageInfo *storageinfodir = nullptr;
    DStorageInfo *storageinfoother = nullptr;
    DUrl url;
    StubExt stl;
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
    DStorageInfo teststoragetemp = teststorage;
    teststoragetemp = *storageinfodir;
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
    stl.set_lamda(g_free,[](){});
    stl.set_lamda(g_object_unref,[](){});
    QProcess::execute("touch "+url.toLocalFile());
    teststorage.setPath(url.toLocalFile());
    EXPECT_FALSE(teststorage.rootPath().isEmpty());
    EXPECT_FALSE(teststorage.device().isEmpty());
    EXPECT_FALSE(teststorage.bytesFree() == 0);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_rootPath) {
    EXPECT_TRUE(storageinfo->rootPath().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    storageinfodir->refresh();
    EXPECT_TRUE(storageinfo->rootPath().isEmpty());
    EXPECT_TRUE(storageinfodir->rootPath().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_device) {
    EXPECT_TRUE(storageinfo->device().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_TRUE(storageinfo->device().isEmpty());
    EXPECT_TRUE(storageinfodir->device().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_fileSystemType) {
    EXPECT_TRUE(storageinfo->fileSystemType().isEmpty());
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_FALSE(storageinfo->fileSystemType().isEmpty());
    storageinfodir->refresh();
    EXPECT_FALSE(storageinfodir->fileSystemType().isEmpty());
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesTotal) {
    EXPECT_TRUE(storageinfo->bytesTotal() != 0);
    storageinfo->setPath("~/test.log");
    storageinfo->refresh();
    EXPECT_TRUE(storageinfo->bytesTotal() != 0);
    EXPECT_FALSE(storageinfodir->bytesTotal() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesFree) {
    EXPECT_TRUE(storageinfo->bytesFree() != 0);
    storageinfo->setPath("~/test.log");
    EXPECT_TRUE(storageinfo->bytesFree() != 0);
    EXPECT_FALSE(storageinfodir->bytesFree() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
}

TEST_F(DStorageInfoTest,can_bytesAvailable) {
    EXPECT_TRUE(storageinfo->bytesAvailable() != 0);
    storageinfo->setPath("~/test.log");
    EXPECT_TRUE(storageinfo->bytesFree() != 0);
    EXPECT_FALSE(storageinfodir->bytesAvailable() == 0);
    url.setScheme(FILE_SCHEME);
    url.setPath("~/test.log");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile());
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
    qDebug() << tmp1;
    EXPECT_TRUE(infolist.contains(tmp));
    EXPECT_TRUE(infolist.contains(tmp1));
    EXPECT_TRUE(infolist.contains(*storageinfodir));
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
    stl.set_lamda(&DStorageInfo::fileSystemType,[](){return QByteArray("avfsd");});
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aclocal"));
    EXPECT_TRUE(DStorageInfo::inSameDevice("/sys/bin/aconnect", "/sys/bin/aconnect"));
    stl.set_lamda(&DStorageInfo::fileSystemType,[](){return QByteArray("gvfsd-fuse");});
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
    DUrl url3,url4;
    url3.setScheme(FILE_SCHEME);
    url4.setScheme(FILE_ROOT);
    EXPECT_FALSE(DStorageInfo::inSameDevice(url3, url4));
    stl.set_lamda(&QUrl::isLocalFile,[](){return false;});
    url4.setScheme(FILE_SCHEME);
    EXPECT_TRUE(DStorageInfo::inSameDevice(url3, url4));
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
