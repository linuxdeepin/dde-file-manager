#include <gtest/gtest.h>
#include <QDateTime>
#include <QProcess>
#include <QSharedPointer>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusReply>

#include "interfaces/dfilesystemmodel.h"
#include "testhelper.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/shareinfoframe.h"
#include "../dde-file-manager-daemon/dbusservice/dbusinterface/usershare_interface.h"
#include "ddialog.h"
#include "dabstractfilewatcher.h"
#define private public
#include "stub.h"
#include "stubext.h"
#include "usershare/usersharemanager.h"
#include "testhelper.h"

using namespace testing;
using namespace stub_ext;
DWIDGET_USE_NAMESPACE
class UserShareManagerTest:public testing::Test{

public:
    QSharedPointer<UserShareManager> sharemanager = nullptr;
    StubExt stl;
    virtual void SetUp() override{
        sharemanager.reset(new UserShareManager());
        std::cout << "start UserShareManagerTest" << std::endl;

        typedef int(*fptr)(QDialog*);
        fptr pQDialogExec = (fptr)(&QDialog::exec);
        int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Rejected;};
        stl.set(pQDialogExec, stub_DDialog_exec);
    }

    virtual void TearDown() override{
        std::cout << "end UserShareManagerTest" << std::endl;
    }
};

TEST_F(UserShareManagerTest,start_initMonitorPath){
    ShareInfoList (*shareInfoList)(void *) = [](void *){
        ShareInfoList infolist;
        ShareInfo info;
        info.setPath("~/Videos");
        infolist << info;
        return infolist;
    };
    stl.set(ADDR(UserShareManager,shareInfoList),shareInfoList);
    ASSERT_NO_FATAL_FAILURE(sharemanager->initMonitorPath());
}

TEST_F(UserShareManagerTest,start_initConnect){
    ASSERT_NO_FATAL_FAILURE(sharemanager->initConnect());
}

TEST_F(UserShareManagerTest,can_getCacehPath){
    EXPECT_FALSE(sharemanager->getCacehPath().isEmpty());
}

TEST_F(UserShareManagerTest,can_getOldShareInfoByNewInfo){
    EXPECT_FALSE(sharemanager->getOldShareInfoByNewInfo(ShareInfo()).isValid());
    ShareInfo info;
    info.setShareName("share_ut_test");
    info.setPath("rand_key");
    sharemanager->m_sharePathToNames.insert("rand_key",QStringList() << "rand_value");
    EXPECT_FALSE(sharemanager->getOldShareInfoByNewInfo(info).isValid());
}

TEST_F(UserShareManagerTest,can_getShareInfoByPath){
    QString (*getShareNameByPath)(const QString &) = [](const QString &){return QString("share_manager");};
    stl.set(ADDR(UserShareManager,getShareNameByPath),getShareNameByPath);
    sharemanager->m_shareInfos.insert("share_manager",ShareInfo());
    EXPECT_FALSE(sharemanager->getShareInfoByPath(QString()).isValid());
}

TEST_F(UserShareManagerTest,can_getShareNameByPath){
    EXPECT_TRUE(sharemanager->getShareNameByPath(QString("rand_key")).isEmpty());
    sharemanager->m_sharePathToNames.insert("rand_key",QStringList() << "rand_value");
    EXPECT_FALSE(sharemanager->getShareNameByPath(QString("rand_key")).isEmpty());
}

TEST_F(UserShareManagerTest,can_shareInfoList){
    sharemanager->m_shareInfos.insert("share_manager",ShareInfo());
    EXPECT_FALSE(sharemanager->shareInfoList().isEmpty());
    ASSERT_NO_FATAL_FAILURE(sharemanager->initSamaServiceSettings());
}

TEST_F(UserShareManagerTest,can_validShareInfoCount){
    ShareInfo info;
    info.setPath("/bin");
    info.setShareName("share_test_manager");
    sharemanager->m_shareInfos.insert("share_manager",info);
    EXPECT_TRUE(sharemanager->validShareInfoCount() != 0);
}

TEST_F(UserShareManagerTest,can_hasValidShareFolders){
    EXPECT_FALSE(sharemanager->hasValidShareFolders());
    ShareInfo info;
    info.setPath("/bin");
    info.setShareName("share_test_manager");
    sharemanager->m_shareInfos.insert("share_manager",info);
    EXPECT_TRUE(sharemanager->hasValidShareFolders());
}

TEST_F(UserShareManagerTest,can_isShareFile){
    EXPECT_FALSE(sharemanager->isShareFile("/share_test_sharemanager"));
}

TEST_F(UserShareManagerTest,can_handleShareChanged){
    ASSERT_NO_FATAL_FAILURE(sharemanager->handleShareChanged("/jfej:tmp"));
    void (*refresh)(const DUrl &) = [](const DUrl &){};
    stl.set(ADDR(DFileSystemModel,refresh),refresh);
    TestHelper::runInLoop([=](){
        ASSERT_NO_FATAL_FAILURE(sharemanager->handleShareChanged("/jfej23"));
    });
}
TEST_F(UserShareManagerTest,can_updateUserShareInfo){
    StubExt stlext;
    typedef bool (*openfile)(QFile *,QFile::OpenMode);
    stlext.set_lamda((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)),[](){return false;});
    ASSERT_NO_FATAL_FAILURE(sharemanager->updateUserShareInfo(false));
    stlext.reset((openfile)((bool (QFile::*)(QFile::OpenMode))(&QFile::open)));

    DUrl dirurl,fileurl,fileurl1;
    dirurl.setScheme(FILE_SCHEME);
    dirurl.setPath("/tmp/ut_sharemange");
    QProcess::execute("mkdir " + dirurl.toLocalFile());
    fileurl.setScheme(FILE_SCHEME);
    fileurl1.setScheme(FILE_SCHEME);
    fileurl.setUrl(dirurl.toString() + "/ut_sharemangaer_1.txt");
    fileurl1.setUrl(dirurl.toString() + "/ut_sharemangaer_2.txt");
    QProcess::execute("touch "+ fileurl.toLocalFile() + " " + fileurl1.toLocalFile());
    QFile file1(fileurl.toLocalFile()),file2(fileurl1.toLocalFile());
    QProcess::execute("mkdir /tmp/ut_sharemange_1 /tmp/ut_sharemange_2");
    if(file1.open(QIODevice::WriteOnly )) {
        file1.write("#VERSION 2\npath=/tmp/ut_sharemange_1\ncomment= \nusershare_acl=S-1-1-0:f\nguest_ok=y\nsharename=ut_share_manager_1");
        file1.close();
    }
    if(file2.open(QIODevice::WriteOnly )) {
        file2.write("#VERSION 2\npath=/tmp/ut_sharemange_2\ncomment= \nusershare_acl=S-1-1-0:r\nguest_ok=y\nsharename=ut_share_manager_2");
        file2.close();
    }
    sharemanager->m_sharePathToNames.insert("/tmp/ut_sharemange_2",QStringList());
    QString (*UserSharePath)(void *) = [](void *){return QString("/tmp/ut_sharemange");};
    stl.set(ADDR(UserShareManager,UserSharePath),UserSharePath);

    ASSERT_NO_FATAL_FAILURE(sharemanager->updateUserShareInfo(false));
    stl.reset(ADDR(UserShareManager,UserSharePath));
    ASSERT_NO_FATAL_FAILURE(sharemanager->updateUserShareInfo(true));
    EXPECT_FALSE(sharemanager->UserSharePath().isEmpty());
    TestHelper::deleteTmpFiles(QStringList() << dirurl.toLocalFile()
                               << "/tmp/ut_sharemange_1" << "/tmp/ut_sharemange_2");
}
TEST_F(UserShareManagerTest,can_setSambaPassword){
    QDBusPendingReply<bool> (*setUserSharePassword)(const QString &, const QString &) = []
            (const QString &, const QString &){
        QDBusMessage reply;
        reply.setArguments(QList<QVariant>() << QVariant(true));
        return QDBusPendingReply<bool>(reply);
    };
    void (*waitForFinished)(void *) = [](void *){};
    stl.set(ADDR(QDBusPendingCall,waitForFinished),waitForFinished);
    stl.set(ADDR(UserShareInterface,setUserSharePassword),setUserSharePassword);
    ASSERT_NO_FATAL_FAILURE(sharemanager->setSambaPassword(QString(),QString()));
    QDBusPendingReply<bool> (*setUserSharePassword1)(const QString &, const QString &) = []
            (const QString &, const QString &){
        QDBusMessage reply;
        reply.setArguments(QList<QVariant>() << QVariant());
        return QDBusPendingReply<bool>(reply);
    };
    stl.set(ADDR(UserShareInterface,setUserSharePassword),setUserSharePassword1);
    ASSERT_NO_FATAL_FAILURE(sharemanager->setSambaPassword(QString(),QString()));
}

TEST_F(UserShareManagerTest,can_addUserShare){

    ShareInfo info;
    DUrl url;
    url.setPath("/tmp/ut_share_manager");
    url.setScheme(FILE_SCHEME);
    TestHelper::deleteTmpFile(url.toLocalFile());
    QProcess::execute("mkdir "+url.toLocalFile());
    info.setPath(url.path());

    info.setShareName("ut_share_manager");
    info.setIsGuestOk(true);
    info.setIsWritable(true);
    StubExt stl;
    void (*showErrorDialog)(const QString &, const QString &) = []
            (const QString &, const QString &){};
    stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);
    void (*showAddUserShareFailedDialog)(const QString &) = []
            (const QString &){};
    stl.set(ADDR(DialogManager,showAddUserShareFailedDialog),showAddUserShareFailedDialog);
    QString (*findExecutable)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){return QString();};
    stl.set(ADDR(QStandardPaths,findExecutable),findExecutable);
    EXPECT_FALSE(sharemanager->addUserShare(info));
    QString (*findExecutable1)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){return QString("net");};
    stl.set(ADDR(QStandardPaths,findExecutable),findExecutable1);
    ShareInfo (*getOldShareInfoByNewInfo)(const ShareInfo &) = [](const ShareInfo &){
        return ShareInfo();
    };
    stl.set(ADDR(UserShareManager,getOldShareInfoByNewInfo),getOldShareInfoByNewInfo);
    EXPECT_FALSE(sharemanager->addUserShare(ShareInfo()));
    ShareInfo (*getOldShareInfoByNewInfo1)(const ShareInfo &) = [](const ShareInfo &){
        ShareInfo info;

        info.setShareName("ut_share_manager");
        info.setPath("/tmp/ut_share_manager");
        return info;
    };
    stl.set(ADDR(UserShareManager,getOldShareInfoByNewInfo),getOldShareInfoByNewInfo1);

    void (*start)(const QString &, const QStringList&, QIODevice::OpenMode) = []
            (const QString &, const QStringList&, QIODevice::OpenMode){

    };
    stl.set((void (QProcess::*)(const QString &,const QStringList &,QIODevice::OpenMode))ADDR(QProcess,start),start);
    bool (*waitForFinished)(int) = [](int){return true;};
    stl.set(ADDR(QProcess,waitForFinished),waitForFinished);
    int (*exitCode)(void *) = [](void *){return 0;};
    stl.set(ADDR(QProcess,exitCode),exitCode);
    EXPECT_TRUE(sharemanager->addUserShare(info));
    int (*exitCode1)(void *) = [](void *){return 1;};
    stl.set(ADDR(QProcess,exitCode),exitCode1);
    QByteArray (*readAllStandardError)(void *) = [](void *){
        return QByteArray("is already a valid system user name");
    };
    stl.set(ADDR(QProcess,readAllStandardError),readAllStandardError);

    EXPECT_FALSE(sharemanager->addUserShare(info));
    info.setIsWritable(false);
    QByteArray (*readAllStandardError1)(void *) = [](void *){
        return QByteArray("as we are restricted to only sharing directories we own.");
    };
    stl.set(ADDR(QProcess,readAllStandardError),readAllStandardError1);
    EXPECT_FALSE(sharemanager->addUserShare(info));

    QByteArray (*readAllStandardError2)(void *) = [](void *){
        return QByteArray("contains invalid characters");
    };
    stl.set(ADDR(QProcess,readAllStandardError),readAllStandardError2);
    EXPECT_FALSE(sharemanager->addUserShare(info));

    QByteArray (*readAllStandardError3)(void *) = [](void *){
        return QByteArray("net usershare add: failed to add share ut_share_manager. Error was");
    };
    stl.set(ADDR(QProcess,readAllStandardError),readAllStandardError3);
    EXPECT_FALSE(sharemanager->addUserShare(info));

    QByteArray (*readAllStandardError4)(void *) = [](void *){
        return QByteArray("__________");
    };
    stl.set(ADDR(QProcess,readAllStandardError),readAllStandardError4);
    EXPECT_FALSE(sharemanager->addUserShare(info));
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << "/tmp/ut_share_manager");
}

TEST_F(UserShareManagerTest,can_deleteUserShareByPath){
    TestHelper::runInLoop([]{});
    QString (*getShareNameByPath)(const QString &) = [](const QString &){return QString("test");};
    stl.set(ADDR(UserShareManager,getShareNameByPath),getShareNameByPath);
    void (*deleteUserShareByShareName)(const QString &) = [](const QString &){};
    stl.set(ADDR(UserShareManager,deleteUserShareByShareName),deleteUserShareByShareName);
    ASSERT_NO_FATAL_FAILURE(sharemanager->deleteUserShareByPath(QString()));
}


TEST_F(UserShareManagerTest,can_removeFiledeleteUserShareByPath){
    QString (*getShareNameByPath)(const QString &) = [](const QString &){
        return QString();
    };
    TestHelper::runInLoop([]{});
    sharemanager->removeFiledeleteUserShareByPath("f");
    stl.set(ADDR(UserShareManager,getShareNameByPath),getShareNameByPath);
    ASSERT_NO_FATAL_FAILURE(sharemanager->removeFiledeleteUserShareByPath("f"));
    QString (*getShareNameByPath1)(const QString &) = [](const QString &){
        return QString("test");
    };
    stl.set(ADDR(UserShareManager,getShareNameByPath),getShareNameByPath1);
    void (*start)(const QString &, const QStringList&, QIODevice::OpenMode) = []
            (const QString &, const QStringList&, QIODevice::OpenMode){};
    stl.set((void (QProcess::*)(const QString &,const QStringList &,QIODevice::OpenMode))ADDR(QProcess,start),start);
    bool (*waitForFinished)(int) = [](int){return true;};
    stl.set(ADDR(QProcess,waitForFinished),waitForFinished);
    ASSERT_NO_FATAL_FAILURE(sharemanager->removeFiledeleteUserShareByPath(QString("test")));
}

TEST_F(UserShareManagerTest,can_usershareCountchanged){
    ASSERT_NO_FATAL_FAILURE(sharemanager->usershareCountchanged());
}

TEST_F(UserShareManagerTest,can_onFileDeleted){
    void (*handleShareChanged)(const QString &) = [](const QString &){};
    stl.set(ADDR(UserShareManager,handleShareChanged),handleShareChanged);
    void (*removeFiledeleteUserShareByPath)(const QString &) = [](const QString &){};
    stl.set(ADDR(UserShareManager,removeFiledeleteUserShareByPath),removeFiledeleteUserShareByPath);
    ASSERT_NO_FATAL_FAILURE(sharemanager->onFileDeleted(QString()));
    ASSERT_NO_FATAL_FAILURE(sharemanager->onFileDeleted(QString("/var/lib/samba/usershares/dd")));
}

TEST_F(UserShareManagerTest,can_deleteUserShareByShareName){
    QDBusPendingReply<bool> (*closeSmbShareByShareName)(const QString &,bool) = []
            (const QString &, bool){
        QDBusMessage reply;
        reply.setArguments(QList<QVariant>() << QVariant(false));
        return QDBusPendingReply<bool>(reply);
    };
    void (*waitForFinished)(void *) = [](void *){};
    stl.set(ADDR(QDBusPendingCall,waitForFinished),waitForFinished);
    stl.set((QDBusPendingReply<bool> (UserShareInterface::*)(const QString &,bool))\
            ADDR(UserShareInterface,closeSmbShareByShareName),closeSmbShareByShareName);
    void (*updateShareInfo)(const QString &) = [](const QString &){};
    stl.set(ADDR(ShareInfoFrame,updateShareInfo),updateShareInfo);
    sharemanager->m_shareInfos.insert("testee",ShareInfo());
    ASSERT_NO_FATAL_FAILURE(sharemanager->deleteUserShareByShareName(QString("testee")));
    QDBusPendingReply<bool> (*closeSmbShareByShareName1)(const QString &,bool) = []
            (const QString &, bool){
        QDBusMessage reply;
        reply.setArguments(QList<QVariant>() << QVariant(true));
        return QDBusPendingReply<bool>(reply);
    };
    stl.set((QDBusPendingReply<bool> (UserShareInterface::*)(const QString &,bool))\
            ADDR(UserShareInterface,closeSmbShareByShareName),closeSmbShareByShareName1);
    void (*start)(const QString &, const QStringList&, QIODevice::OpenMode) = []
            (const QString &, const QStringList&, QIODevice::OpenMode){};
    stl.set((void (QProcess::*)(const QString &,const QStringList &,QIODevice::OpenMode))ADDR(QProcess,start),start);
    bool (*waitForFinished1)(int) = [](int){return true;};
    stl.set(ADDR(QProcess,waitForFinished),waitForFinished1);
    sharemanager->m_shareInfos.clear();
    ASSERT_NO_FATAL_FAILURE(sharemanager->deleteUserShareByShareName(QString("testee")));
}

TEST_F(UserShareManagerTest,can_loadUserShareInfoPathNames){
    QString (*readCacheFromFile)(const QString &) = [](const QString &){return QString("rwwa");};
    stl.set(ADDR(UserShareManager,readCacheFromFile),readCacheFromFile);
    ASSERT_NO_FATAL_FAILURE(sharemanager->loadUserShareInfoPathNames());
    QString (*readCacheFromFile1)(const QString &) = [](const QString &){
        return QString("{\"ut_share_test1\":{}}");
    };
    stl.set(ADDR(UserShareManager,readCacheFromFile),readCacheFromFile1);
    ASSERT_NO_FATAL_FAILURE(sharemanager->loadUserShareInfoPathNames());
}

TEST_F(UserShareManagerTest,can_saveUserShareInfoPathNames){
    void (*writeCacheToFile)(const QString &, const QString &) = [](const QString &, const QString &){};

    stl.set(ADDR(UserShareManager,writeCacheToFile),writeCacheToFile);
    sharemanager->m_sharePathByFilePath.insert("ut_share_manger",QString());
    ASSERT_NO_FATAL_FAILURE(sharemanager->saveUserShareInfoPathNames());
}

TEST_F(UserShareManagerTest,can_updateFileAttributeInfo){
    bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
            (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
    stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
            ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
    ASSERT_NO_FATAL_FAILURE(sharemanager->updateFileAttributeInfo(QString()));
    ASSERT_NO_FATAL_FAILURE(sharemanager->updateFileAttributeInfo(QString("~/")));
}

TEST_F(UserShareManagerTest,can_writeCacheToFile){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    ASSERT_NO_FATAL_FAILURE(sharemanager->writeCacheToFile(url.toLocalFile(),"mniah"));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(UserShareManagerTest,can_readCacheFromFile){
    EXPECT_TRUE(sharemanager->readCacheFromFile("file:///utueutndfnsndfh").isEmpty());
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    EXPECT_TRUE(sharemanager->readCacheFromFile(url.toLocalFile()).isEmpty());
    TestHelper::deleteTmpFile(url.toLocalFile());
    TestHelper::deleteTmpFile("/tmp/ut_share_manager");
}
