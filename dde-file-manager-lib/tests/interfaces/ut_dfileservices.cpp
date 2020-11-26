#include <gtest/gtest.h>
#include <interfaces/dfileservices.h>
#include <QSharedPointer>
#include "controllers/appcontroller.h"
#include "controllers/networkcontroller.h"
#include "controllers/trashmanager.h"
#include <QThread>
#include "testhelper.h"

using namespace testing;
DFM_USE_NAMESPACE

class DFileSeviceTest:public testing::Test{

public:

    DFileService *service = nullptr;
    virtual void SetUp() override{
        service = DFileService::instance();
        urlvideos.setScheme(FILE_SCHEME);
        urlvideos.setPath("~/Videos");
        std::cout << "start DFileSeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        QProcess::execute("killall dde-file-manager");
        QProcess::execute("killall deepin-editor");
        std::cout << "end DFileSeviceTest" << std::endl;
    }

    DUrl urlvideos;
};

TEST_F(DFileSeviceTest, can_isRegisted){
    AppController::instance()->registerUrlHandle();
    EXPECT_TRUE(service->isRegisted(SMB_SCHEME,"",typeid (NetworkController)));
    EXPECT_TRUE(service->isRegisted (TRASH_SCHEME,""));
    service->initHandlersByCreators();
    DFileService::printStacktrace();
    DFileService::printStacktrace(3);
}

TEST_F(DFileSeviceTest, start_HandlerOp){
    service->clearFileUrlHandler(TRASH_SCHEME,"");
    auto *tempTrashMgr = new TrashManager();
    tempTrashMgr->setObjectName("trashMgr");
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);
    DFileService::unsetFileUrlHandler(tempTrashMgr);
    delete tempTrashMgr;
    AppController::instance()->registerUrlHandle();
    EXPECT_FALSE( DFileService::getHandlerTypeByUrl(urlvideos).isEmpty());
}

TEST_F(DFileSeviceTest, start_openFileOp){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    EXPECT_TRUE(service->openFile(nullptr,urlvideos));
    EXPECT_TRUE(service->openFiles(nullptr,DUrlList() << urlvideos << DUrl()));
    QString filepath = TestHelper::createTmpFile(".txt");
    url.setPath(filepath);
    TestHelper::runInLoop([=](){
        EXPECT_FALSE(service->openFileByApp(nullptr,"deepin-editor",url));
    });
    TestHelper::runInLoop([=](){
        EXPECT_FALSE(service->openFilesByApp(nullptr,"deepin-editor",DUrlList() << url << urlvideos));
    });
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_compressOp){
//    DUrl url,url1;
//    url.setScheme(FILE_SCHEME);
//    url.setPath("~/Videos");
//    EXPECT_TRUE(service->compressFiles(nullptr,DUrlList() << url));
//    EXPECT_TRUE(service->compressFiles(nullptr,DUrlList() << url));
//    EXPECT_TRUE(service->compressFiles(nullptr,DUrlList() << url));
}

TEST_F(DFileSeviceTest, start_writeFilesToClipboard){
    EXPECT_TRUE(service->writeFilesToClipboard(nullptr,DFMGlobal::UnknowAction,DUrlList() << urlvideos));
}

TEST_F(DFileSeviceTest, start_renameFile){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile();
    url.setPath(filepath);
    to = url;
    to.setPath(filepath+"_1");
    EXPECT_TRUE(service->renameFile(nullptr,url,to));
    TestHelper::deleteTmpFile(to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_multiFilesReplaceName){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile("_qq_ut_test");
    url.setPath(filepath);
    EXPECT_TRUE(service->multiFilesReplaceName(DUrlList() << url,QPair<QString,QString>("_qq_ut_test","_ww_ut_test")));
    filepath = filepath.replace("_qq_ut_test","_ww_ut_test");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << filepath);
}

TEST_F(DFileSeviceTest, start_multiFilesAddStrToName){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile("_qq_ut_test");
    url.setPath(filepath);
    EXPECT_TRUE(service->multiFilesAddStrToName(DUrlList() << url,QPair<QString,DFileService::AddTextFlags>("_ww",DFileService::AddTextFlags::After)));
    filepath+="_ww";
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << filepath);
}

TEST_F(DFileSeviceTest, start_multiFilesCustomName){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile("_qq_ut_test"));
//    EXPECT_TRUE(service->multiFilesCustomName(DUrlList() << url,QPair<QString,QString>("_qq","_ww")));
//    to.setPath("./" + filename + "_ww.txt");
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_deleteFiles){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile("_qq_ut_test"));
//    QProcess::execute("chmod 0777 "+filepath);
//    EXPECT_TRUE(service->deleteFiles(nullptr,DUrlList() << url,true));
//    QProcess::execute("touch "+url.toLocalFile());
    DUrlList rmd;
    rmd = service->moveToTrash(nullptr,DUrlList() << url);
    EXPECT_FALSE(rmd.isEmpty());
    EXPECT_FALSE(service->restoreFile(nullptr,rmd));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_pasteFileByClipboard){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpDir();
    url.setPath(filepath);
    service->pasteFileByClipboard(nullptr,url);
    TestHelper::deleteTmpFile(filepath);
}

TEST_F(DFileSeviceTest, start_pasteFile){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpDir();
    url.setPath(TestHelper::createTmpDir());
    to.setScheme(FILE_SCHEME);
    to.setPath(TestHelper::createTmpFile(".txt"));
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->pasteFile(nullptr,DFMGlobal::CopyAction,url,DUrlList() << to).isEmpty());
    });
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_fileOperations){
    DUrl url,to,linkurl,testfileinfo,destinfo;
    url.setScheme(FILE_SCHEME);
    QString filename = "ut_test_defileservice";
    QString filepath = "./" + filename;
    url.setPath(filepath);
    EXPECT_TRUE(service->mkdir(nullptr,url));
    to = url;
    to.setPath("./"+filename+".txt");
    linkurl = to;
    linkurl.setPath("./"+filename+"sys.txt");
    EXPECT_TRUE(service->touchFile(nullptr,to));
    EXPECT_TRUE(service->openFileLocation(nullptr,url));
    EXPECT_TRUE(service->setPermissions(nullptr,to,QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    EXPECT_FALSE(service->addToBookmark(nullptr,to));
    EXPECT_TRUE(service->removeBookmark(nullptr,to));
//    EXPECT_TRUE(service->createSymlink(nullptr,to));
    EXPECT_TRUE(service->createSymlink(nullptr,to,linkurl));
    destinfo = to;
    destinfo.setPath("~/Desktop/"+filename+" Shortcut.txt");
//    EXPECT_TRUE(service->sendToDesktop(nullptr,DUrlList() << to));
    service->sendToBluetooth(DUrlList() << to);
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->shareFolder(nullptr,url,"ut_share_test"));
    });
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->unShareFolder(nullptr,url));
    });
    EXPECT_TRUE(service->openInTerminal(nullptr,url));
    EXPECT_TRUE(service->setFileTags(nullptr,to,QStringList() << "ut_tag_test"));
    EXPECT_TRUE(service->makeTagsOfFiles(nullptr,DUrlList() << to,QStringList() << "ut_tag_test"));
    EXPECT_FALSE(service->getTagsThroughFiles(nullptr,DUrlList() << to).isEmpty());
    EXPECT_TRUE(service->removeTagsOfFile(nullptr,to,QStringList() << "ut_tag_test"));
    testfileinfo = to;
    testfileinfo.setPath("~/Videos");
    EXPECT_TRUE(service->createFileInfo(nullptr,testfileinfo));
    EXPECT_TRUE(service->createFileInfo(nullptr,testfileinfo));
    EXPECT_TRUE(service->createDirIterator(nullptr,url,QStringList(),QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden,
                                           QDirIterator::NoIteratorFlags));
    EXPECT_TRUE(service->getChildren(Q_NULLPTR, url, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden).isEmpty());
    EXPECT_TRUE(QSharedPointer<JobController>(DFileService::instance()->getChildrenJob(nullptr, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                                                            QDirIterator::NoIteratorFlags, true, false)));
    EXPECT_TRUE(QSharedPointer<DAbstractFileWatcher>(service->createFileWatcher(nullptr,url)));
    EXPECT_TRUE(service->setExtraProperties(nullptr,testfileinfo,QVariantHash()));
    EXPECT_TRUE(QSharedPointer<DFileDevice>(service->createFileDevice(nullptr,to)));
    EXPECT_TRUE(QSharedPointer<DFileHandler>(service->createFileHandler(nullptr,to)));
    EXPECT_TRUE(QSharedPointer<DStorageInfo>(service->createStorageInfo(nullptr,to)));

    TestHelper::runInLoop([=](){
        service->moveToTrash(nullptr,DUrlList() << destinfo << linkurl << to << url);
    });
}

TEST_F(DFileSeviceTest, start_otherOperations){
    service->setCursorBusyState(true);
    service->setCursorBusyState(true);
    service->setCursorBusyState(false);
    service->setCursorBusyState(false);
    DUrl to(urlvideos);
    EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos,false));
    urlvideos.setUrl("file:///run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,false));
    urlvideos.setUrl("dfmroot:///%252Frun%252Fuser%252F1000%252Fgvfs%252Fftp%253Ahost%253D10.8.0.116.gvfsmp");
    EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,false));
    EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,"ftp:host=10.8.0.116",false));
    EXPECT_TRUE(service->isNetWorkOnline());
    service->setDoClearTrashState(true);
    EXPECT_TRUE(service->getDoClearTrashState());
    service->setDoClearTrashState(false);
    EXPECT_FALSE(service->getDoClearTrashState());
//    service->dealPasteEnd(dMakeEventPointer<DFMEvent>(nullptr),DUrlList() << to);
    EXPECT_TRUE(service->isSmbFtpContain(to));
    service->onTagEditorChanged(QStringList() << "tag_ut_test", DUrlList() << to);
    EXPECT_TRUE(service->removeTagsOfFile(nullptr,to,QStringList() << "ut_tag_test"));
}
