#include <gtest/gtest.h>
#include <QSharedPointer>
#include <QFile>
#include <QThread>
#include <QFileDialog>

#include "controllers/appcontroller.h"
#include "controllers/networkcontroller.h"
#include "controllers/trashmanager.h"
#include "controllers/vaultcontroller.h"
#include "interfaces/plugins/dfmfilecontrollerfactory.h"
#include "dialogs/dialogmanager.h"
#include "controllers/trashmanager.h"
#include "stubext.h"
#include "tag/tagmanager.h"
#include "ddesktopservices.h"
#include "usershare/usersharemanager.h"
#include "fileoperations/filejob.h"
#include "dabstractfilewatcher.h"
#define private public
#include "testhelper.h"
#include "stub.h"
#include "shutil/checknetwork.h"
#include "shutil/fileutils.h"
#include "interfaces/dfmeventdispatcher.h"
#include "interfaces/dfmabstracteventhandler.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "controllers/pathmanager.h"
#include "controllers/filecontroller.h"
#include "interfaces/dfileservices.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
class DFileSeviceTest:public testing::Test{

public:

    DFileService *service = nullptr;
    virtual void SetUp() override{
        QStringList (*keys)(void *) = [](void *){
            return QStringList() << "/tmp/ut_dfileservice_controller";
        };
        stl.set(ADDR(DFMFileControllerFactory,keys),keys);
        service = DFileService::instance();
        urlvideos.setScheme(FILE_SCHEME);
        urlvideos.setPath("~/Videos");
        DFMEventFuture (*processEventAsync)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
                (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
            return DFMEventFuture(QFuture<QVariant>());
        };

        stl.set((DFMEventFuture(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher,processEventAsync),processEventAsync);
        void (*showNewWindow)(const DUrl &, const bool &) = [](const DUrl &, const bool &){return;};
        stl.set(ADDR(WindowManager,showNewWindow),showNewWindow);
        bool (*cd)(const DUrl &) = [](const DUrl &){return false;};
        stl.set(ADDR(DFileManagerWindow,cd),cd);

        void (*showErrorDialog)(const QString &, const QString &) = []
                (const QString &, const QString &){};
        stl.set(ADDR(DialogManager,showErrorDialog),showErrorDialog);
        void (*showUnableToLocateDir)(const QString &) = []
                (const QString &){};
        stl.set(ADDR(DialogManager,showUnableToLocateDir),showUnableToLocateDir);
        void (*showRenameNameSameErrorDialog)(const QString &,const DFMEvent &) = []
                (const QString &,const DFMEvent &){};
        stl.set(ADDR(DialogManager,showRenameNameSameErrorDialog),showRenameNameSameErrorDialog);

        bool (*ghostSignal)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
                (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int){return true;};
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
                ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal);
        bool (*ghostSignal1)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
                (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &){return true;};
        bool (*ghostSignal2)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &) = []
                (const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &){return true;};
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
                ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal1);
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2 , const DUrl &, const DUrl &))\
                ADDR(DAbstractFileWatcher,ghostSignal),ghostSignal2);
        std::cout << "start DFileSeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileSeviceTest" << std::endl;
    }

    DUrl urlvideos;
    StubExt stl;
};

TEST_F(DFileSeviceTest, sart_fmevent){
    DUrl url,to,urlrename;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile(".txt"));
    to = url;
    to.setPath(TestHelper::createTmpDir());
    void (*showNewWindow)(const DUrl &, const bool &) = [](const DUrl &, const bool &){return;};
    stl.set(ADDR(WindowManager,showNewWindow),showNewWindow);
    bool (*cd)(const DUrl &) = [](const DUrl &){return false;};
    stl.set(ADDR(DFileManagerWindow,cd),cd);

    bool (*openFile)(const QString &) = [](const QString &){return true;};
    stl.set(ADDR(FileUtils,openFile),openFile);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileEvent>(nullptr, url),nullptr));
    bool (*openFilesByApp)(const QString &, const QStringList &) = []
            (const QString &, const QStringList &){
        return true;
    };
    stl.set(ADDR(FileUtils,openFilesByApp),openFilesByApp);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileByAppEvent>
                                 (nullptr, "/usr/share/applications/deepin-editor.desktop", url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCompressEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDecompressEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDecompressHereEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    QString urlpath = url.toLocalFile();
    QString topath = to.toLocalFile();
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>
                                 (nullptr, DFMGlobal::CopyAction, DUrlList()),nullptr));

    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false),nullptr));


    url.setPath(to.toLocalFile()+"/tmpj");
    to.setPath("/tmp");
    int (*showRenameNameDotDotErrorDialog)(const DFMEvent &) = [](const DFMEvent &){return 0;};
    stl.set(ADDR(DialogManager,showRenameNameDotDotErrorDialog),
            showRenameNameDotDotErrorDialog);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false),nullptr));
    url.setPath(urlpath);
    to.setPath("/tmp/ut_dfileservice_rename.txt");
    bool (*rename)(const QString &) = [](const QString &){return true;};
    stl.set((bool (QFile::*)(const QString &))ADDR(QFile,rename),rename);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false),nullptr));
    url.setPath("/tmp/ut_dfileservice_rename.txt");
    to.setPath(topath);
    bool (*deleteTags)(const QList<QString> &) = [](const QList<QString> &){return true;};
    stl.set(ADDR(TagManager,deleteTags),deleteTags);
    url.setScheme(BOOKMARK_SCHEME);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << url, false, false),nullptr));
    DUrlList (*pasteFilesV2)(const QSharedPointer<DFMPasteEvent> &, DFMGlobal::ClipboardAction ,
                          const DUrlList &, const DUrl &, bool, bool, bool ) = []
            (const QSharedPointer<DFMPasteEvent> &, DFMGlobal::ClipboardAction ,
            const DUrlList &, const DUrl &, bool, bool, bool){
        return DUrlList() << DUrl();
    };
    stl.set(ADDR(FileController,pasteFilesV2),pasteFilesV2);
    url.setScheme(FILE_SCHEME);
    url.setPath("/tmp/ut_dfileserviece_test");
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << url, false, false),nullptr));
    url.setPath(urlpath);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << to, true, false),nullptr));
    url.setUrl(TRASH_ROOT);
    void (*cleanTrash)(const QObject *) = [](const QObject *){};
    stl.set(ADDR(TrashManager,cleanTrash),cleanTrash);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << DUrl::fromTrashFile("/"), true, false),nullptr));
    url.setScheme(FILE_SCHEME);
    bool (*isSystemPath)(QString) = [](QString){
        return true;
    };
    stl.set(ADDR(PathManager,isSystemPath),isSystemPath);
    void (*showDeleteSystemPathWarnDialog)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DialogManager,showDeleteSystemPathWarnDialog),showDeleteSystemPathWarnDialog);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMoveToTrashEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    stl.reset(ADDR(PathManager,isSystemPath));
    url.setPath(urlpath);
    DUrlList (*doMoveToTrash)(const DUrlList &) = [](const DUrlList &){
        return DUrlList() << DUrl();
    };
    stl.set(ADDR(FileJob,doMoveToTrash),doMoveToTrash);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMoveToTrashEvent>
                                 (nullptr, DUrlList() << to << DUrl()),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRestoreFromTrashEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMPasteEvent>
                                 (nullptr, DFMGlobal::CopyAction, url, DUrlList() << to),nullptr));
    bool (*mkpath)(const QString &) = [](const QString &){return true;};
    stl.set(ADDR(QDir,mkpath),mkpath);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMkdirEvent>
                                 (nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMTouchFileEvent>
                                 (nullptr, url),nullptr));
    bool (*showFileItem)(QUrl, const QString &) = [](QUrl, const QString &){return true;};
    stl.set((bool (*)(QUrl, const QString &))\
            ADDR(DDesktopServices,showFileItem),showFileItem);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileLocation>
                                 (nullptr, url),nullptr));
    bool (*touchFile)(const QObject *, const DUrl &) = []
            (const QObject *, const DUrl &){return true;};
    stl.set(ADDR(DFileService,touchFile),touchFile);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMAddToBookmarkEvent>
                                 (nullptr, url),nullptr));
    stl.reset(ADDR(DFileService,touchFile));

    bool (*deleteFiles)(const QObject *, const DUrlList &, bool , bool , bool ) = []
            (const QObject *, const DUrlList &, bool , bool , bool ){return true;};
    stl.set(ADDR(DFileService,deleteFiles),deleteFiles);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRemoveBookmarkEvent>
                                 (nullptr, url),nullptr));
    stl.reset(ADDR(DFileService,deleteFiles));

    bool (*link)(const QString &) = [](const QString &){return true;};
    stl.set((bool (QFile::*)(const QString &))ADDR(QFile,link),link);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateSymlinkEvent>
                                 (nullptr, url, to, false),nullptr));
    bool (*addUserShare)(const ShareInfo &) = [](const ShareInfo &){return true;};
    stl.set(ADDR(UserShareManager,addUserShare),addUserShare);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMFileShareEvent>
                                 (nullptr, url, "share_test", false, false),nullptr));
    void (*deleteUserShareByPath)(const QString &) = [](const QString &){};
    stl.set(ADDR(UserShareManager,deleteUserShareByPath),deleteUserShareByPath);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCancelFileShareEvent>
                                 (nullptr, url),nullptr));
    bool (*startDetached)(const QString &) = [](const QString &){return true;};
    stl.set((bool (*)(const QString &))ADDR(QProcess,startDetached),startDetached);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenInTerminalEvent>
                                 (nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMGetChildrensEvent>
                                 (nullptr, url, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                                  QDirIterator::NoIteratorFlags, false, false),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateFileInfoEvent>
                                 (nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateDiriterator>
                                 (nullptr,url,QStringList(),QDir::AllEntries | QDir::System
                                  | QDir::NoDotAndDotDot | QDir::Hidden,
                                  QDirIterator::NoIteratorFlags, false, false),nullptr));
    const auto &&event =dMakeEventPointer<DFMCreateGetChildrensJob>(nullptr, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                                                                    QDirIterator::NoIteratorFlags, true, false);
    EXPECT_TRUE(service->fmEvent(event,nullptr));
    event->setAccepted(false);
    EXPECT_TRUE(service->fmEvent(event,nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateFileWatcherEvent>
                                 (nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMSetFileExtraProperties>
                                 (nullptr, url, QVariantHash()),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateFileDevice, nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateFileHandler, nullptr, url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateStorageInfo, nullptr, url),nullptr));
    bool (*removeTagsOfFiles)(const QList<QString> &, const QList<DUrl> &) = []
            (const QList<QString> &, const QList<DUrl> &){
        return true;
    };
    stl.set(ADDR(TagManager,removeTagsOfFiles),removeTagsOfFiles);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRemoveTagsOfFileEvent>
                                 (nullptr, url, QStringList()),nullptr));
    QList<QString> (*getTagsThroughFiles)(
                const QObject *, const QList<DUrl> &, const bool) = [](
            const QObject *, const QList<DUrl> &, const bool){
        QList<QString> tt;
        tt << "ooo";
        return tt;
    };
    stl.set(ADDR(DFileService,getTagsThroughFiles),getTagsThroughFiles);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMGetTagsThroughFilesEvent>
                                 (nullptr, DUrlList() << url),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMSetPermissionEvent>
                                 (nullptr, url, QFileDevice::ReadOwner),nullptr));
    bool (*openFiles)(const QStringList &) = [](const QStringList &){return true;};
    stl.set(ADDR(FileUtils,openFiles),openFiles);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFilesEvent>
                                 (nullptr, DUrlList() << url, false),nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFilesByAppEvent>
                                 (nullptr, "deepin-editor", DUrlList() << url, false),nullptr));
    EXPECT_FALSE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url),nullptr));
    url.setScheme(FILE_SCHEME);
    url.setPath(urlpath);
    to.setScheme(FILE_SCHEME);
    to.setPath(topath);
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileSeviceTest, can_isRegisted){
    AppController::instance()->registerUrlHandle();
    EXPECT_TRUE(service->isRegisted(SMB_SCHEME,"",typeid (NetworkController)));
    EXPECT_TRUE(service->isRegisted (TRASH_SCHEME,""));
    ASSERT_NO_FATAL_FAILURE(service->initHandlersByCreators());
    DFileService::printStacktrace();
    DFileService::printStacktrace(3);
}

TEST_F(DFileSeviceTest, start_HandlerOp){
    ASSERT_NO_FATAL_FAILURE(service->clearFileUrlHandler(TRASH_SCHEME,""));
    auto *tempTrashMgr = new TrashManager();
    tempTrashMgr->setObjectName("trashMgr");
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);
    DFileService::unsetFileUrlHandler(tempTrashMgr);
    DFileService::unsetFileUrlHandler(tempTrashMgr);
    delete tempTrashMgr;
    AppController::instance()->registerUrlHandle();
    EXPECT_FALSE( DFileService::getHandlerTypeByUrl(urlvideos).isEmpty());
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);
}

TEST_F(DFileSeviceTest, start_openFileOp){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(false);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);

    EXPECT_FALSE(service->openFile(nullptr,urlvideos));
    EXPECT_FALSE(service->openFiles(nullptr,DUrlList() << urlvideos << DUrl()));
    QString filepath = TestHelper::createTmpFile(".txt");
    url.setPath(filepath);
    EXPECT_FALSE(service->openFileByApp(nullptr,"deepin-editor",url));
    EXPECT_FALSE(service->openFilesByApp(nullptr,"deepin-editor",DUrlList() << url << urlvideos));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_compressOp){
    DUrl url,url1;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(false);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_FALSE(service->compressFiles(nullptr,DUrlList() << url));
    EXPECT_FALSE(service->decompressFile(nullptr,DUrlList() << url));
    EXPECT_FALSE(service->decompressFileHere(nullptr,DUrlList() << url));
}

TEST_F(DFileSeviceTest, start_writeFilesToClipboard){
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(true);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
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
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile("_qq_ut_test"));
    EXPECT_FALSE(service->multiFilesCustomName(DUrlList() << url,QPair<QString,QString>("_qq","_ww")));
    to.setPath(url.toLocalFile().replace("_qq","_ww"));
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_deleteFiles){
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("./_qq_ut_test");
    QProcess::execute("touch " + url.toLocalFile());
    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);

    EXPECT_FALSE(service->deleteFiles(nullptr,DUrlList(),true));

    EXPECT_FALSE(service->deleteFiles(nullptr,DUrlList() << url,true));
    bool (*isSystemPath)(QString) = [](QString){return true;};
    stl.set(ADDR(PathManager,isSystemPath),isSystemPath);
    void (*showDeleteSystemPathWarnDialog)(quint64 ) = [](quint64 ){};
    stl.set(ADDR(DialogManager,showDeleteSystemPathWarnDialog),showDeleteSystemPathWarnDialog);
    EXPECT_FALSE(service->deleteFiles(nullptr,DUrlList() << url,true));

    DUrlList rmd;
    EXPECT_TRUE(service->moveToTrash(nullptr,DUrlList()).isEmpty());
    bool (*isGvfsMountFile)(const QString &, const bool &) = [](const QString &, const bool &){return true;};
    stl.set(ADDR(FileUtils,isGvfsMountFile),isGvfsMountFile);
    EXPECT_FALSE(service->moveToTrash(nullptr,DUrlList() << url).isEmpty());
    stl.reset(ADDR(FileUtils,isGvfsMountFile));

    QList<QUrl> (*clipboardFileUrlList)(void *) = [](void *){
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath(QFileInfo("./_qq_ut_test").absoluteFilePath());
        return QList<QUrl>() << url;
    };
    DFMGlobal::ClipboardAction (*clipboardAction)(void *) = [](void *){ return DFMGlobal::CopyAction;};
    stl.set(ADDR(DFMGlobal,clipboardFileUrlList),clipboardFileUrlList);
    stl.set(ADDR(DFMGlobal,clipboardAction),clipboardAction);
    EXPECT_TRUE(service->moveToTrash(nullptr,DUrlList() << url).isEmpty());

    QList<QUrl> (*clipboardFileUrlList1)(void *) = [](void *){
        QUrl url,url1;
        url.setScheme(FILE_SCHEME);
        url.setPath("./_qq_ut_test");
        url1 = url;
        url1.setPath("/usr/bin");
        return QList<QUrl>() << url << url1;
    };
    stl.set(ADDR(DFMGlobal,clipboardFileUrlList),clipboardFileUrlList1);
    EXPECT_TRUE(service->moveToTrash(nullptr,DUrlList() << url).isEmpty());
    DFMGlobal::instance()->clearClipboard();
    EXPECT_FALSE(service->restoreFile(nullptr,rmd));
    TestHelper::deleteTmpFile(url.toLocalFile());

}

TEST_F(DFileSeviceTest, start_pasteFileByClipboard){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpDir();
    url.setPath(filepath);

    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);
    DFMGlobal::ClipboardAction (*clipboardAction)(void *) = [](void *){ return DFMGlobal::UnknowAction;};
    stl.set(ADDR(DFMGlobal,clipboardAction),clipboardAction);
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr,url));

    url.setScheme(SEARCH_SCHEME);
    stl.reset(ADDR(DFMGlobal,clipboardAction));
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr,url));
    TestHelper::runInLoop([=](){});
    url.setScheme(FILE_SCHEME);
    DFMGlobal::ClipboardAction (*clipboardAction1)(void *) = [](void *){ return DFMGlobal::CutAction;};
    stl.set(ADDR(DFMGlobal,clipboardAction),clipboardAction1);
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr,url));
    TestHelper::deleteTmpFile(filepath);
}

TEST_F(DFileSeviceTest, start_pasteFile){
    DUrl url,to;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpDir());
    to.setScheme(FILE_SCHEME);
    to.setPath(TestHelper::createTmpFile(".txt"));
    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    TestHelper::runInLoop([=](){});
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);
    EXPECT_TRUE(service->pasteFile(nullptr,DFMGlobal::CopyAction,url,DUrlList() << to).isEmpty());
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_fileOperations){
    TestHelper::runInLoop([=](){});
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
    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);
    EXPECT_TRUE(service->setPermissions(nullptr,to,QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    QVariant (*processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(true);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_TRUE(service->openFileLocation(nullptr,url));
    EXPECT_TRUE(service->addToBookmark(nullptr,to));
    EXPECT_TRUE(service->removeBookmark(nullptr,to));
    stl.reset((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent));
    QString (*getSaveFileName)(QWidget *,
                                       const QString &,
                                       const QString &,
                                       const QString &,
                                       QString *,
                                       QFileDialog::Options) = [](QWidget *,
            const QString &,
            const QString &,
            const QString &,
            QString *,
            QFileDialog::Options){
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath("./ut_test_defileservice_sys.txt");
        return url.toLocalFile();
    };
    stl.set(ADDR(QFileDialog,getSaveFileName),getSaveFileName);
    QString (*getSymlinkFileName)(const DUrl &, const QDir &) = [](const DUrl &, const QDir &){
        return QString();
    };
    stl.set(ADDR(DFileService,getSymlinkFileName),getSymlinkFileName);
    bool (*isVaultFile)(void *) = [](void *){return true;};
    stl.set(ADDR(VaultController,isVaultFile),isVaultFile);
    void (*showFailToCreateSymlinkDialog)(const QString &) = [](const QString &){};
    stl.set(ADDR(DialogManager,showFailToCreateSymlinkDialog),showFailToCreateSymlinkDialog);
    EXPECT_FALSE(service->createSymlink(nullptr,to));
    stl.reset(ADDR(VaultController,isVaultFile));
    QString (*getSaveFileName1)(QWidget *,
                                       const QString &,
                                       const QString &,
                                       const QString &,
                                       QString *,
                                       QFileDialog::Options) = [](QWidget *,
            const QString &,
            const QString &,
            const QString &,
            QString *,
            QFileDialog::Options){
        return QString();
    };
    stl.set(ADDR(QFileDialog,getSaveFileName),getSaveFileName1);
    EXPECT_FALSE(service->createSymlink(nullptr,to));
    stl.reset(ADDR(QFileDialog,getSaveFileName));
    stl.reset(ADDR(DFileService,getSymlinkFileName));
    EXPECT_TRUE(service->createSymlink(nullptr,to,linkurl));
    destinfo = to;
    destinfo.setPath("~/Desktop/ut_test_sendtodesktop");
    QString (*writableLocation)(QStandardPaths::StandardLocation) = [](QStandardPaths::StandardLocation){
        return QString();
    };
    QString (*writableLocation1)(QStandardPaths::StandardLocation) = [](QStandardPaths::StandardLocation){
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath("~/Desktop/ut_test_sendtodesktop");
        return url.toLocalFile();
    };
    stl.set(ADDR(QStandardPaths,writableLocation),writableLocation);
    EXPECT_FALSE(service->sendToDesktop(nullptr,DUrlList() << to));
    stl.set(ADDR(QStandardPaths,writableLocation),writableLocation1);
    EXPECT_FALSE(service->sendToDesktop(nullptr,DUrlList() << to));
    void (*showBluetoothTransferDlg)(const DUrlList &) = [](const DUrlList &){};
    stl.set(ADDR(DialogManager,showBluetoothTransferDlg),showBluetoothTransferDlg);
    ASSERT_NO_FATAL_FAILURE(service->sendToBluetooth(DUrlList() << to));
    stl.reset(ADDR(QStandardPaths,writableLocation));

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEvent),processEvent);
    EXPECT_TRUE(service->shareFolder(nullptr,url,"ut_share_test"));
    EXPECT_TRUE(service->unShareFolder(nullptr,url));

    EXPECT_TRUE(service->openInTerminal(nullptr,url));
    EXPECT_TRUE(service->setFileTags(nullptr,to,QStringList() << "ut_tag_test"));
    stl.reset((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
              ADDR(DFMEventDispatcher,processEvent));
    QList<QString> (*getTagsThroughFiles)(
                const QObject *, const QList<DUrl> &, const bool) = [](
            const QObject *, const QList<DUrl> &, const bool){
        QList<QString> tt;
        tt << "ooo";
        return tt;
    };
    stl.set(ADDR(DFileService,getTagsThroughFiles),getTagsThroughFiles);
    EXPECT_FALSE(service->makeTagsOfFiles(nullptr,DUrlList() << to,QStringList() << "ut_tag_test"));
    stl.reset(ADDR(DFileService,getTagsThroughFiles));
    EXPECT_TRUE(service->getTagsThroughFiles(nullptr,DUrlList() << to).isEmpty());
    EXPECT_TRUE(service->getTagsThroughFiles(nullptr,DUrlList() << to,true).isEmpty());
    EXPECT_TRUE(service->removeTagsOfFile(nullptr,to,QStringList() << "ut_tag_test"));
    testfileinfo = to;
    testfileinfo.setPath("~/Videos");
    EXPECT_TRUE(service->createFileInfo(nullptr,testfileinfo));
    EXPECT_TRUE(service->createFileInfo(nullptr,testfileinfo));
    EXPECT_TRUE(service->createDirIterator(nullptr,url,QStringList(),QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden,
                                           QDirIterator::NoIteratorFlags));
    EXPECT_TRUE(service->createDirIterator(nullptr,url,QStringList(),QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden,
                                           static_cast<QDirIterator::IteratorFlag>(DDirIterator::SortINode)));
    EXPECT_TRUE(service->getChildren(Q_NULLPTR, url, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden).isEmpty());
    EXPECT_TRUE(QSharedPointer<JobController>(DFileService::instance()->getChildrenJob(nullptr, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                                                            QDirIterator::NoIteratorFlags, true, false)));
    EXPECT_TRUE(QSharedPointer<DAbstractFileWatcher>(service->createFileWatcher(nullptr,url)));
    EXPECT_TRUE(service->setExtraProperties(nullptr,testfileinfo,QVariantHash()));
    EXPECT_TRUE(QSharedPointer<DFileDevice>(service->createFileDevice(nullptr,to)));
    EXPECT_TRUE(QSharedPointer<DFileHandler>(service->createFileHandler(nullptr,to)));
    EXPECT_TRUE(QSharedPointer<DStorageInfo>(service->createStorageInfo(nullptr,to)));

    TestHelper::deleteTmpFiles(QStringList() << destinfo.toLocalFile()
                               << linkurl.toLocalFile() << to.toLocalFile() << url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_otherOperations){
    TestHelper::runInLoop([=](){});
    QVariant (*processEventWithEventLoop)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher,processEventWithEventLoop),processEventWithEventLoop);
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(true));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(true));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(false));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(false));
    DUrl to(urlvideos);
    urlvideos.setUrl("dfmroot:///sda");
    EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos,true));
    urlvideos.setUrl("file:///run/user/1000/gvfs/afc:host=d7b4e5ea332532a868cf8eb2d64d5e443aa76981,port=3");
    EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos,true));
    TestHelper::runInLoop([=](){
        EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos,true));
    });
    urlvideos.setUrl("file:///run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,true));
    });
    urlvideos.setUrl("dfmroot:///%252Frun%252Fuser%252F1000%252Fgvfs%252Fftp%253Ahost%253D10.8.70.116.gvfsmp");
    Stub st;
    bool (*isHostAndPortConnect)(const QString &, const QString &) = [](const QString &, const QString &){return false;};
    st.set(ADDR(CheckNetwork,isHostAndPortConnect),isHostAndPortConnect);
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,true));
    });
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,"ftp:host=10.8.70.110",true));
    });
    EXPECT_FALSE(service->checkGvfsMountfileBusy(DUrl(),"ftp:host=10.8.70.110",true));
    EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,"",true));
    TestHelper::runInLoop([=](){
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos,"ftp:host=10.8.70.110",true));
    });
    TestHelper::runInLoop([=](){
        EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos,"ftp:host=10.8.70.110,21",true));
    });
    ASSERT_NO_FATAL_FAILURE(service->setDoClearTrashState(true));
    EXPECT_TRUE(service->getDoClearTrashState());
    ASSERT_NO_FATAL_FAILURE(service->setDoClearTrashState(false));
    EXPECT_FALSE(service->getDoClearTrashState());

    auto ecenttmp = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction,
                                                     DUrl::fromLocalFile(to.toLocalFile()), DUrlList() << to);
    ecenttmp->setData(QVariant::fromValue(DUrlList() << to));
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp,DUrlList() << to));
    ecenttmp = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction,
                                                         DUrl::fromLocalFile(to.toLocalFile()), DUrlList() << to);
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp,DUrlList() << to));
    int (*execute)(const QString &) = [](const QString &) {return 0;};
    stl.set((int (*)(const QString &))ADDR(QProcess,execute),execute);
    to.setPath("~/.cache/deepin/discburn/_dev_sr1/tesatj");
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp,DUrlList() << to));

    EXPECT_FALSE(service->isSmbFtpContain(to));
    TestHelper::runInLoop([=](){
        ASSERT_NO_FATAL_FAILURE(service->onTagEditorChanged(QStringList() << "tag_ut_test", DUrlList() << to));
    });
    TestHelper::runInLoop([=]{
        EXPECT_TRUE(service->removeTagsOfFile(nullptr,to,QStringList() << "ut_tag_test"));
    });
}

TEST_F(DFileSeviceTest, start_laterRequestSelectFiles){
    ASSERT_NO_FATAL_FAILURE(service->laterRequestSelectFiles(DFMUrlListBaseEvent(nullptr, DUrlList())));
}

TEST_F(DFileSeviceTest, start_insertToCreatorHash){
    QUrl qurl;
    ASSERT_NO_FATAL_FAILURE(service->insertToCreatorHash(HandlerType(qurl.scheme(), qurl.host()),
                                 HandlerCreatorType(typeid(DFMFileControllerFactory).name(), [] {
        return DFMFileControllerFactory::create("key");
    })));
}

TEST_F(DFileSeviceTest, start_getSymlinkFileName){
    EXPECT_TRUE(service->getSymlinkFileName(DUrl()).isEmpty());
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    EXPECT_FALSE(service->getSymlinkFileName(url).isEmpty());
    QStringList deletefilepath;
    deletefilepath << url.toLocalFile();
    url.setPath(TestHelper::createTmpFile(".txt"));
    EXPECT_FALSE(service->getSymlinkFileName(url).isEmpty());
    deletefilepath << url.toLocalFile();
    url.setPath(TestHelper::createTmpDir());
    EXPECT_FALSE(service->getSymlinkFileName(url).isEmpty());
    QString (*path)(void *) = [](void *){return QString();};
    stl.set(ADDR(QDir,path),path);
    EXPECT_FALSE(service->getSymlinkFileName(url).isEmpty());
    deletefilepath << url.toLocalFile();
    url.setPath("/bin");
    EXPECT_FALSE(service->getSymlinkFileName(url).isEmpty());
    TestHelper::deleteTmpFiles( deletefilepath);
}

TEST_F(DFileSeviceTest, start_checkMultiSelectionFilesCache){
    EXPECT_FALSE(service->checkMultiSelectionFilesCache());
}
