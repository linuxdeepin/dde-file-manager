/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liygang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#include "ddialog.h"
#include "dfiledevice.h"
#include "dfilehandler.h"
#include "controllers/jobcontroller.h"
#include "dstorageinfo.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
class DFileSeviceTest: public testing::Test
{

public:

    DFileService *service = nullptr;
    virtual void SetUp() override
    {
        QStringList(*keyslamda)(void *) = [](void *) {
            return QStringList() << "/tmp/ut_dfileservice_controller";
        };
        stl.set(ADDR(DFMFileControllerFactory, keys), keyslamda);
        service = DFileService::instance();
        urlvideos.setScheme(FILE_SCHEME);
        urlvideos.setPath("~/Videos");
        DFMEventFuture(*processEventAsynclamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
        (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
            return DFMEventFuture(QFuture<QVariant>());
        };

        stl.set((DFMEventFuture(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventAsync), processEventAsynclamda);
        void (*showNewWindowlamda)(const DUrl &, const bool &) = [](const DUrl &, const bool &) {return;};
        stl.set(ADDR(WindowManager, showNewWindow), showNewWindowlamda);
        bool (*cdlamda)(const DUrl &) = [](const DUrl &) {return false;};
        stl.set(ADDR(DFileManagerWindow, cd), cdlamda);

        void (*showErrorDialoglamda)(const QString &, const QString &) = []
        (const QString &, const QString &) {};
        stl.set(ADDR(DialogManager, showErrorDialog), showErrorDialoglamda);
        void (*showUnableToLocateDirlamda)(const QString &) = []
        (const QString &) {};
        stl.set(ADDR(DialogManager, showUnableToLocateDir), showUnableToLocateDirlamda);
        void (*showRenameNameSameErrorDialoglamda)(const QString &, const DFMEvent &) = []
        (const QString &, const DFMEvent &) {};
        stl.set(ADDR(DialogManager, showRenameNameSameErrorDialog), showRenameNameSameErrorDialoglamda);

        bool (*ghostSignallamda)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) = []
        (const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int) {return true;};
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType3, const DUrl &, const int))\
                ADDR(DAbstractFileWatcher, ghostSignal), ghostSignallamda);
        bool (*ghostSignal1lamda)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) = []
        (const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &) {return true;};
        bool (*ghostSignal2lamda)(const DUrl &, DAbstractFileWatcher::SignalType2, const DUrl &, const DUrl &) = []
        (const DUrl &, DAbstractFileWatcher::SignalType2, const DUrl &, const DUrl &) {return true;};
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType1, const DUrl &))\
                ADDR(DAbstractFileWatcher, ghostSignal), ghostSignal1lamda);
        stl.set((bool (*)(const DUrl &, DAbstractFileWatcher::SignalType2, const DUrl &, const DUrl &))\
                ADDR(DAbstractFileWatcher, ghostSignal), ghostSignal2lamda);
        std::cout << "start DFileSeviceTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end DFileSeviceTest" << std::endl;
    }

    DUrl urlvideos;
    StubExt stl;
};

TEST_F(DFileSeviceTest, sart_fmevent)
{
    TestHelper::runInLoop([]() {});
    DUrl url, to, urlrename;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile(".txt"));
    to = url;
    to.setPath(TestHelper::createTmpDir());
    void (*showNewWindowlamda)(const DUrl &, const bool &) = [](const DUrl &, const bool &) {return;};
    stl.set(ADDR(WindowManager, showNewWindow), showNewWindowlamda);
    bool (*cdlamda)(const DUrl &) = [](const DUrl &) {return false;};
    stl.set(ADDR(DFileManagerWindow, cd), cdlamda);

    bool (*openFilelamda)(const QString &) = [](const QString &) {return true;};
    stl.set(ADDR(FileUtils, openFile), openFilelamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileEvent>(nullptr, url), nullptr));
    bool (*openFilesByApplamda)(const QString &, const QStringList &) = []
    (const QString &, const QStringList &) {
        return true;
    };
    stl.set(ADDR(FileUtils, openFilesByApp), openFilesByApplamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileByAppEvent>
                                 (nullptr, "/usr/share/applications/deepin-editor.desktop", url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCompressEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDecompressEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDecompressHereEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    QString urlpath = url.toLocalFile();
    QString topath = to.toLocalFile();
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>
                                 (nullptr, DFMGlobal::CopyAction, DUrlList()), nullptr));

    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false), nullptr));


    url.setPath(to.toLocalFile() + "/tmpj");
    to.setPath("/tmp");
    int (*showRenameNameDotDotErrorDialoglamda)(const DFMEvent &) = [](const DFMEvent &) {return 0;};
    stl.set(ADDR(DialogManager, showRenameNameDotDotErrorDialog),
            showRenameNameDotDotErrorDialoglamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false), nullptr));
    url.setPath(urlpath);
    to.setPath("/tmp/ut_dfileservice_rename.txt");
    bool (*rename)(const QString &) = [](const QString &) {return true;};
    stl.set((bool (QFile::*)(const QString &))ADDR(QFile, rename), rename);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRenameEvent>
                                 (nullptr, url, to, false), nullptr));
    url.setPath("/tmp/ut_dfileservice_rename.txt");
    to.setPath(topath);
    bool (*deleteTagslamda)(const QList<QString> &) = [](const QList<QString> &) {return true;};
    stl.set(ADDR(TagManager, deleteTags), deleteTagslamda);
    url.setScheme(BOOKMARK_SCHEME);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << url, false, false), nullptr));
    DUrlList(*pasteFilesV2lamda)(const QSharedPointer<DFMPasteEvent> &, DFMGlobal::ClipboardAction,
                                 const DUrlList &, const DUrl &, bool, bool, bool) = []
                                                                                     (const QSharedPointer<DFMPasteEvent> &, DFMGlobal::ClipboardAction,
    const DUrlList &, const DUrl &, bool, bool, bool) {
        return DUrlList() << DUrl();
    };
    stl.set(ADDR(FileController, pasteFilesV2), pasteFilesV2lamda);
    url.setScheme(FILE_SCHEME);
    url.setPath("/tmp/ut_dfileserviece_test");
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << url, false, false), nullptr));
    url.setPath(urlpath);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << to, true, false), nullptr));
    url.setUrl(TRASH_ROOT);
    void (*cleanTrashlamda)(const QObject *) = [](const QObject *) {};
    stl.set(ADDR(TrashManager, cleanTrash), cleanTrashlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMDeleteEvent>
                                 (nullptr, DUrlList() << DUrl::fromTrashFile("/"), true, false), nullptr));
    url.setScheme(FILE_SCHEME);
    bool (*isSystemPathlamda)(QString) = [](QString) {
        return true;
    };
    stl.set(ADDR(PathManager, isSystemPath), isSystemPathlamda);
    void (*showDeleteSystemPathWarnDialoglamda)(quint64) = [](quint64) {};
    stl.set(ADDR(DialogManager, showDeleteSystemPathWarnDialog), showDeleteSystemPathWarnDialoglamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMoveToTrashEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    stl.reset(ADDR(PathManager, isSystemPath));
    url.setPath(urlpath);
    DUrlList(*doMoveToTrashlamda)(const DUrlList &) = [](const DUrlList &) {
        return DUrlList() << DUrl();
    };
    stl.set(ADDR(FileJob, doMoveToTrash), doMoveToTrashlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMoveToTrashEvent>
                                 (nullptr, DUrlList() << to << DUrl()), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRestoreFromTrashEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMPasteEvent>
                                 (nullptr, DFMGlobal::CopyAction, url, DUrlList() << to), nullptr));
    bool (*mkpathlamda)(const QString &) = [](const QString &) {return true;};
    stl.set(ADDR(QDir, mkpath), mkpathlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMMkdirEvent>
                                 (nullptr, url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMTouchFileEvent>
                                 (nullptr, url), nullptr));
    bool (*showFileItemlamda)(QUrl, const QString &) = [](QUrl, const QString &) {return true;};
    stl.set((bool (*)(QUrl, const QString &))\
            ADDR(DDesktopServices, showFileItem), showFileItemlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFileLocation>
                                 (nullptr, url), nullptr));
    bool (*touchFilelamda)(const QObject *, const DUrl &) = []
    (const QObject *, const DUrl &) {return true;};
    stl.set(ADDR(DFileService, touchFile), touchFilelamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMAddToBookmarkEvent>
                                 (nullptr, url), nullptr));
    stl.reset(ADDR(DFileService, touchFile));

    bool (*deleteFileslamda)(const QObject *, const DUrlList &, bool, bool, bool) = []
    (const QObject *, const DUrlList &, bool, bool, bool) {return true;};
    stl.set(ADDR(DFileService, deleteFiles), deleteFileslamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRemoveBookmarkEvent>
                                 (nullptr, url), nullptr));
    stl.reset(ADDR(DFileService, deleteFiles));

    bool (*linklamda)(const QString &) = [](const QString &) {return true;};
    stl.set((bool (QFile::*)(const QString &))ADDR(QFile, link), linklamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateSymlinkEvent>
                                 (nullptr, url, to, false), nullptr));
    bool (*addUserSharelamda)(const ShareInfo &) = [](const ShareInfo &) {return true;};
    stl.set(ADDR(UserShareManager, addUserShare), addUserSharelamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMFileShareEvent>
                                 (nullptr, url, "share_test", false, false), nullptr));
    void (*deleteUserShareByPathlamda)(const QString &) = [](const QString &) {};
    stl.set(ADDR(UserShareManager, deleteUserShareByPath), deleteUserShareByPathlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCancelFileShareEvent>
                                 (nullptr, url), nullptr));
    bool (*startDetachedlamda)(const QString &) = [](const QString &) {return true;};
    stl.set((bool (*)(const QString &))ADDR(QProcess, startDetached), startDetachedlamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenInTerminalEvent>
                                 (nullptr, url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMGetChildrensEvent>
                                 (nullptr, url, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
                                  QDirIterator::NoIteratorFlags, false, false), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateFileInfoEvent>
                                 (nullptr, url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateDiriterator>
                                 (nullptr, url, QStringList(), QDir::AllEntries | QDir::System
                                  | QDir::NoDotAndDotDot | QDir::Hidden,
                                  QDirIterator::NoIteratorFlags, false, false), nullptr));
    const auto &&event = dMakeEventPointer<DFMCreateGetChildrensJob>(nullptr, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                                                                     QDirIterator::NoIteratorFlags, true, false);
    QVariant getChildrensJob;
    EXPECT_TRUE(service->fmEvent(event, &getChildrensJob));
    auto *job1 = getChildrensJob.value<JobController *>();
    FreePointer(job1);

    event->setAccepted(false);
    EXPECT_TRUE(service->fmEvent(event, &getChildrensJob));
    auto *job2 = getChildrensJob.value<JobController *>();
    FreePointer(job2);

    QVariant fileWatcher;
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMCreateFileWatcherEvent>
                                 (nullptr, url), &fileWatcher));
    auto *watcher = fileWatcher.value<DAbstractFileWatcher *>();
    FreePointer(watcher);

    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMSetFileExtraProperties>
                                 (nullptr, url, QVariantHash()), nullptr));
    QVariant fileDevice;
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateFileDevice, nullptr, url), &fileDevice));
    DFileDevice *device = fileDevice.value<DFileDevice *>();
    FreePointer(device);

    QVariant fileHandle;
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateFileHandler, nullptr, url), &fileHandle));
    DFileHandler *handle = fileHandle.value<DFileHandler *>();
    FreePointer(handle);

    QVariant storageInfo;
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>
                                 (DFMEvent::CreateStorageInfo, nullptr, url), &storageInfo));
    auto *info = storageInfo.value<DStorageInfo *>();
    FreePointer(info);

    bool (*removeTagsOfFileslamda)(const QList<QString> &, const QList<DUrl> &) = []
    (const QList<QString> &, const QList<DUrl> &) {
        return true;
    };
    stl.set(ADDR(TagManager, removeTagsOfFiles), removeTagsOfFileslamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMRemoveTagsOfFileEvent>
                                 (nullptr, url, QStringList()), nullptr));
    QList<QString> (*getTagsThroughFileslamda)(
        const QObject *, const QList<DUrl> &, const bool) = [](
    const QObject *, const QList<DUrl> &, const bool) {
        QList<QString> tt;
        tt << "ooo";
        return tt;
    };
    stl.set(ADDR(DFileService, getTagsThroughFiles), getTagsThroughFileslamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMGetTagsThroughFilesEvent>
                                 (nullptr, DUrlList() << url), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMSetPermissionEvent>
                                 (nullptr, url, QFileDevice::ReadOwner), nullptr));
    bool (*openFileslamda)(const QStringList &) = [](const QStringList &) {return true;};
    stl.set(ADDR(FileUtils, openFiles), openFileslamda);
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFilesEvent>
                                 (nullptr, DUrlList() << url, false), nullptr));
    EXPECT_TRUE(service->fmEvent(dMakeEventPointer<DFMOpenFilesByAppEvent>
                                 (nullptr, "deepin-editor", DUrlList() << url, false), nullptr));
    EXPECT_FALSE(service->fmEvent(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url), nullptr));
    url.setScheme(FILE_SCHEME);
    url.setPath(urlpath);
    to.setScheme(FILE_SCHEME);
    to.setPath(topath);
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

/* 该用例存在高风险异步操作 暂时注释
TEST_F(DFileSeviceTest, can_isRegisted)
{
    AppController::instance()->registerUrlHandle();
    EXPECT_TRUE(service->isRegisted(SMB_SCHEME, "", typeid(NetworkController)));
    EXPECT_TRUE(service->isRegisted(TRASH_SCHEME, ""));
    ASSERT_NO_FATAL_FAILURE(service->initHandlersByCreators());
    DFileService::printStacktrace();
    DFileService::printStacktrace(3);

    service->clearController(FILE_SCHEME, "");
    service->clearController(SEARCH_SCHEME, "");
    service->clearController(NETWORK_SCHEME, "");
    service->clearController(SMB_SCHEME, "");
    service->clearController(SFTP_SCHEME, "");
    service->clearController(FTP_SCHEME, "");
    service->clearController(DAV_SCHEME, "");
    service->clearController(USERSHARE_SCHEME, "");
    service->clearController(AVFS_SCHEME, "");
    service->clearController(MOUNT_SCHEME, "");
    service->clearController(BURN_SCHEME, "");
    service->clearController(TAG_SCHEME, "");
    service->clearController(RECENT_SCHEME, "");
    service->clearController(DFMMD_SCHEME, "");
    service->clearController(DFMROOT_SCHEME, "");
    service->clearController(DFMVAULT_SCHEME, "");
}*/

TEST_F(DFileSeviceTest, start_HandlerOp)
{
    ASSERT_NO_FATAL_FAILURE(service->clearFileUrlHandler(TRASH_SCHEME, ""));
    auto *tempTrashMgr = new TrashManager();
    tempTrashMgr->setObjectName("trashMgr");
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);
    //DFileService::unsetFileUrlHandler(tempTrashMgr);
    DFileService::unsetFileUrlHandler(tempTrashMgr);
    delete tempTrashMgr;
    AppController::instance()->registerUrlHandle();
    EXPECT_FALSE(DFileService::getHandlerTypeByUrl(urlvideos).isEmpty());
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);

    service->clearController(urlvideos.scheme(), urlvideos.host());
}

TEST_F(DFileSeviceTest, start_openFileOp)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    QVariant(*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(false);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEvent), processEventlamda);

    EXPECT_FALSE(service->openFile(nullptr, urlvideos));
    EXPECT_FALSE(service->openFiles(nullptr, DUrlList() << urlvideos << DUrl()));
    QString filepath = TestHelper::createTmpFile(".txt");
    url.setPath(filepath);
    EXPECT_FALSE(service->openFileByApp(nullptr, "deepin-editor", url));
    EXPECT_FALSE(service->openFilesByApp(nullptr, "deepin-editor", DUrlList() << url << urlvideos));
    TestHelper::deleteTmpFile(url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_compressOp)
{
    DUrl url, url1;
    url.setScheme(FILE_SCHEME);
    url.setPath("~/Videos");
    QVariant(*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(false);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEvent), processEventlamda);
    EXPECT_FALSE(service->compressFiles(nullptr, DUrlList() << url));
    EXPECT_FALSE(service->decompressFile(nullptr, DUrlList() << url));
    EXPECT_FALSE(service->decompressFileHere(nullptr, DUrlList() << url));
}

TEST_F(DFileSeviceTest, start_writeFilesToClipboard)
{
    QVariant(*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(true);
    };
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEvent), processEventlamda);
    EXPECT_TRUE(service->writeFilesToClipboard(nullptr, DFMGlobal::UnknowAction, DUrlList() << urlvideos));
}

TEST_F(DFileSeviceTest, start_renameFile)
{
    Stub stl;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&Dtk::Widget::DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    DUrl url, to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile();
    url.setPath(filepath);
    to = url;
    to.setPath(filepath + "_1");
    EXPECT_NO_FATAL_FAILURE(service->renameFile(nullptr, url, to));
    TestHelper::deleteTmpFile(to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_multiFilesReplaceName)
{
    DUrl url, to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile("_qq_ut_test");
    url.setPath(filepath);
    EXPECT_NO_FATAL_FAILURE(service->multiFilesReplaceName(DUrlList() << url, QPair<QString, QString>("_qq_ut_test", "_ww_ut_test")));
    filepath = filepath.replace("_qq_ut_test", "_ww_ut_test");
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << filepath);

}

TEST_F(DFileSeviceTest, start_multiFilesAddStrToName)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpFile("_qq_ut_test");
    url.setPath(filepath);
    EXPECT_NO_FATAL_FAILURE(service->multiFilesAddStrToName(DUrlList() << url, QPair<QString, DFileService::AddTextFlags>("_ww", DFileService::AddTextFlags::After)));
    filepath += "_ww";
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << filepath);

}

TEST_F(DFileSeviceTest, start_multiFilesCustomName)
{
    DUrl url, to;
    QVariant(*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEvent), processEventlamda);
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile("_qq_ut_test"));
    EXPECT_FALSE(service->multiFilesCustomName(DUrlList() << url, QPair<QString, QString>("_qq", "_ww")));
    to.setPath(url.toLocalFile().replace("_qq", "_ww"));
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_deleteFiles)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("./_qq_ut_test");
    QProcess::execute("touch " + url.toLocalFile());
    QVariant(*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);

    EXPECT_FALSE(service->deleteFiles(nullptr, DUrlList(), true));

    EXPECT_FALSE(service->deleteFiles(nullptr, DUrlList() << url, true));
    bool (*isSystemPathlamda)(QString) = [](QString) {return true;};
    stl.set(ADDR(PathManager, isSystemPath), isSystemPathlamda);
    void (*showDeleteSystemPathWarnDialoglamda)(quint64) = [](quint64) {};
    stl.set(ADDR(DialogManager, showDeleteSystemPathWarnDialog), showDeleteSystemPathWarnDialoglamda);
    EXPECT_FALSE(service->deleteFiles(nullptr, DUrlList() << url, true));

    DUrlList rmd;
    EXPECT_TRUE(service->moveToTrash(nullptr, DUrlList()).isEmpty());
    bool (*isGvfsMountFilelamda)(const QString &, const bool &) = [](const QString &, const bool &) {return true;};
    stl.set(ADDR(FileUtils, isGvfsMountFile), isGvfsMountFilelamda);
    EXPECT_FALSE(service->moveToTrash(nullptr, DUrlList() << url).isEmpty());
    stl.reset(ADDR(FileUtils, isGvfsMountFile));

    QList<QUrl> (*clipboardFileUrlListlamda)(void *) = [](void *) {
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath(QFileInfo("./_qq_ut_test").absoluteFilePath());
        return QList<QUrl>() << url;
    };
    DFMGlobal::ClipboardAction(*clipboardAction)(void *) = [](void *) { return DFMGlobal::CopyAction;};
    stl.set(ADDR(DFMGlobal, clipboardFileUrlList), clipboardFileUrlListlamda);
    stl.set(ADDR(DFMGlobal, clipboardAction), clipboardAction);
    EXPECT_TRUE(service->moveToTrash(nullptr, DUrlList() << url).isEmpty());

    QList<QUrl> (*clipboardFileUrlList1lamda)(void *) = [](void *) {
        QUrl url, url1;
        url.setScheme(FILE_SCHEME);
        url.setPath("./_qq_ut_test");
        url1 = url;
        url1.setPath("/usr/bin");
        return QList<QUrl>() << url << url1;
    };
    stl.set(ADDR(DFMGlobal, clipboardFileUrlList), clipboardFileUrlList1lamda);
    EXPECT_TRUE(service->moveToTrash(nullptr, DUrlList() << url).isEmpty());
    DFMGlobal::instance()->clearClipboard();
    EXPECT_FALSE(service->restoreFile(nullptr, rmd));
    TestHelper::deleteTmpFile(url.toLocalFile());

}

TEST_F(DFileSeviceTest, start_pasteFileByClipboard)
{
    TestHelper::runInLoop([]() {});
    DUrl url, to;
    url.setScheme(FILE_SCHEME);
    QString filepath = TestHelper::createTmpDir();
    url.setPath(filepath);

    QVariant(*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
    DFMGlobal::ClipboardAction(*clipboardActionlamda)(void *) = [](void *) { return DFMGlobal::UnknowAction;};
    stl.set(ADDR(DFMGlobal, clipboardAction), clipboardActionlamda);
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr, url));

    url.setScheme(SEARCH_SCHEME);
    stl.reset(ADDR(DFMGlobal, clipboardAction));
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr, url));
    url.setScheme(FILE_SCHEME);
    DFMGlobal::ClipboardAction(*clipboardAction1lamda)(void *) = [](void *) { return DFMGlobal::CutAction;};
    stl.set(ADDR(DFMGlobal, clipboardAction), clipboardAction1lamda);
    ASSERT_NO_FATAL_FAILURE(service->pasteFileByClipboard(nullptr, url));
    TestHelper::deleteTmpFile(filepath);
}

TEST_F(DFileSeviceTest, start_pasteFile)
{
    DUrl url, to;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpDir());
    to.setScheme(FILE_SCHEME);
    to.setPath(TestHelper::createTmpFile(".txt"));
    QVariant(*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    TestHelper::runInLoop([ = ]() {});
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
    EXPECT_TRUE(service->pasteFile(nullptr, DFMGlobal::CopyAction, url, DUrlList() << to).isEmpty());
    TestHelper::deleteTmpFiles(QStringList() << url.toLocalFile() << to.toLocalFile());
}

TEST_F(DFileSeviceTest, start_fileOperations)
{
    TestHelper::runInLoop([ = ]() {});
    DUrl url, to, linkurl, testfileinfo, destinfo;
    url.setScheme(FILE_SCHEME);
    QString filename = "ut_test_defileservice";
    QString filepath = "./" + filename;
    url.setPath(filepath);
    EXPECT_NO_FATAL_FAILURE(service->mkdir(nullptr, url));
    to = url;
    to.setPath("./" + filename + ".txt");
    linkurl = to;
    linkurl.setPath("./" + filename + "sys.txt");
    EXPECT_NO_FATAL_FAILURE(service->touchFile(nullptr, to));
    typedef int(*fptr)(QDialog *);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);
    QVariant(*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        EXPECT_NO_FATAL_FAILURE(service->setPermissions(nullptr, to, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
    }
    QVariant(*processEventlamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(true);
    };
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEvent), processEventlamda);
        EXPECT_NO_FATAL_FAILURE(service->openFileLocation(nullptr, url));
        EXPECT_NO_FATAL_FAILURE(service->addToBookmark(nullptr, to));
        EXPECT_NO_FATAL_FAILURE(service->removeBookmark(nullptr, to));
    }
    QString(*getSaveFileNamelamda)(QWidget *,
                                   const QString &,
                                   const QString &,
                                   const QString &,
                                   QString *,
                                   QFileDialog::Options) = [](QWidget *,
                                                              const QString &,
                                                              const QString &,
                                                              const QString &,
                                                              QString *,
    QFileDialog::Options) {
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath("./ut_test_defileservice_sys.txt");
        return url.toLocalFile();
    };
    QString(*getSymlinkFileNamelamda)(const DUrl &, const QDir &) = [](const DUrl &, const QDir &) {
        return QString();
    };
    bool (*isVaultFilelamda)(void *) = [](void *) {return true;};
    void (*showFailToCreateSymlinkDialoglamda)(const QString &) = [](const QString &) {};
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set(ADDR(QFileDialog, getSaveFileName), getSaveFileNamelamda);

        stl.set(ADDR(DFileService, getSymlinkFileName), getSymlinkFileNamelamda);

        stl.set(ADDR(VaultController, isVaultFile), isVaultFilelamda);

        stl.set(ADDR(DialogManager, showFailToCreateSymlinkDialog), showFailToCreateSymlinkDialoglamda);
        EXPECT_NO_FATAL_FAILURE(service->createSymlink(nullptr, to));
    }

    QString(*getSaveFileName1lamda)(QWidget *,
                                    const QString &,
                                    const QString &,
                                    const QString &,
                                    QString *,
                                    QFileDialog::Options) = [](QWidget *,
                                                               const QString &,
                                                               const QString &,
                                                               const QString &,
                                                               QString *,
    QFileDialog::Options) {
        return QString();
    };
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set(ADDR(DFileService, getSymlinkFileName), getSymlinkFileNamelamda);
        stl.set(ADDR(QFileDialog, getSaveFileName), getSaveFileName1lamda);
        EXPECT_NO_FATAL_FAILURE(service->createSymlink(nullptr, to));
    }
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        EXPECT_NO_FATAL_FAILURE(service->createSymlink(nullptr, to, linkurl));
    }
    destinfo = to;
    destinfo.setPath("~/Desktop/ut_test_sendtodesktop");
    QString(*writableLocationlamda)(QStandardPaths::StandardLocation) = [](QStandardPaths::StandardLocation) {
        return QString();
    };
    QString(*writableLocation1lamda)(QStandardPaths::StandardLocation) = [](QStandardPaths::StandardLocation) {
        QUrl url;
        url.setScheme(FILE_SCHEME);
        url.setPath("~/Desktop/ut_test_sendtodesktop");
        return url.toLocalFile();
    };
    void (*showBluetoothTransferDlglamda)(const DUrlList &) = [](const DUrlList &) {};
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set(ADDR(QStandardPaths, writableLocation), writableLocationlamda);
        EXPECT_NO_FATAL_FAILURE(service->sendToDesktop(nullptr, DUrlList() << to));
        stl.set(ADDR(QStandardPaths, writableLocation), writableLocation1lamda);
        EXPECT_NO_FATAL_FAILURE(service->sendToDesktop(nullptr, DUrlList() << to));

        stl.set(ADDR(DialogManager, showBluetoothTransferDlg), showBluetoothTransferDlglamda);
        ASSERT_NO_FATAL_FAILURE(service->sendToBluetooth(DUrlList() << to));
    }

    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEvent), processEventlamda);
        stl.set(ADDR(DialogManager, showBluetoothTransferDlg), showBluetoothTransferDlglamda);
        EXPECT_NO_FATAL_FAILURE(service->shareFolder(nullptr, url, "ut_share_test"));
        EXPECT_NO_FATAL_FAILURE(service->unShareFolder(nullptr, url));

        EXPECT_NO_FATAL_FAILURE(service->openInTerminal(nullptr, url));
        EXPECT_NO_FATAL_FAILURE(service->setFileTags(nullptr, to, QStringList() << "ut_tag_test"));
    }
    QList<QString> (*getTagsThroughFileslamda)(
        const QObject *, const QList<DUrl> &, const bool) = [](
    const QObject *, const QList<DUrl> &, const bool) {
        QList<QString> tt;
        tt << "ooo";
        return tt;
    };
    {
        StubExt stl;
        stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
                ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
        stl.set(ADDR(DialogManager, showBluetoothTransferDlg), showBluetoothTransferDlglamda);
        stl.set(ADDR(DFileService, getTagsThroughFiles), getTagsThroughFileslamda);
        EXPECT_NO_FATAL_FAILURE(service->makeTagsOfFiles(nullptr, DUrlList() << to, QStringList() << "ut_tag_test"));
    }

    StubExt stl;
    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);
    stl.set(ADDR(DialogManager, showBluetoothTransferDlg), showBluetoothTransferDlglamda);
    EXPECT_NO_FATAL_FAILURE(service->getTagsThroughFiles(nullptr, DUrlList() << to).isEmpty());
    EXPECT_NO_FATAL_FAILURE(service->getTagsThroughFiles(nullptr, DUrlList() << to, true).isEmpty());
    EXPECT_NO_FATAL_FAILURE(service->removeTagsOfFile(nullptr, to, QStringList() << "ut_tag_test"));
    testfileinfo = to;
    testfileinfo.setPath("~/Videos");
    EXPECT_NO_FATAL_FAILURE(service->createFileInfo(nullptr, testfileinfo));
    EXPECT_NO_FATAL_FAILURE(service->createFileInfo(nullptr, testfileinfo));
    EXPECT_NO_FATAL_FAILURE(service->createDirIterator(nullptr, url, QStringList(), QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden,
                                           QDirIterator::NoIteratorFlags));
    EXPECT_NO_FATAL_FAILURE(service->createDirIterator(nullptr, url, QStringList(), QDir::AllEntries | QDir::System
                                           | QDir::NoDotAndDotDot | QDir::Hidden,
                                           static_cast<QDirIterator::IteratorFlag>(DDirIterator::SortINode)));
    EXPECT_NO_FATAL_FAILURE(service->getChildren(Q_NULLPTR, url, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden).isEmpty());
    EXPECT_NO_FATAL_FAILURE(QSharedPointer<JobController>(DFileService::instance()->getChildrenJob(nullptr, url, QStringList(), QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot,
                                                                                       QDirIterator::NoIteratorFlags, true, false)));
    EXPECT_NO_FATAL_FAILURE(QSharedPointer<DAbstractFileWatcher>(service->createFileWatcher(nullptr, url)));
    EXPECT_NO_FATAL_FAILURE(service->setExtraProperties(nullptr, testfileinfo, QVariantHash()));
    EXPECT_NO_FATAL_FAILURE(QSharedPointer<DFileDevice>(service->createFileDevice(nullptr, to)));
    EXPECT_NO_FATAL_FAILURE(QSharedPointer<DFileHandler>(service->createFileHandler(nullptr, to)));
    EXPECT_NO_FATAL_FAILURE(QSharedPointer<DStorageInfo>(service->createStorageInfo(nullptr, to)));

    TestHelper::deleteTmpFiles(QStringList() << destinfo.toLocalFile()
                               << linkurl.toLocalFile() << to.toLocalFile() << url.toLocalFile());
}

TEST_F(DFileSeviceTest, start_otherOperations)
{
    /*TestHelper::runInLoop([ = ]() {});
    QVariant(*processEventWithEventLooplamda)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
    (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };

    stl.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEventWithEventLoop), processEventWithEventLooplamda);

    typedef int(*fptr)(QDialog *);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stl.set(pQDialogExec, stub_DDialog_exec);
    stl.set(pDDialogExec, stub_DDialog_exec);

    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(true));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(true));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(false));
    ASSERT_NO_FATAL_FAILURE(service->setCursorBusyState(false));
    DUrl to(urlvideos);
    urlvideos.setUrl("dfmroot:///sda");
    EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos, true));
    urlvideos.setUrl("file:///run/user/1000/gvfs/afc:host=d7b4e5ea332532a868cf8eb2d64d5e443aa76981,port=3");
    EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos, true));
    TestHelper::runInLoop([ = ]() {
        EXPECT_FALSE(service->checkGvfsMountfileBusy(urlvideos, true));
    });
    urlvideos.setUrl("file:///run/user/1000/gvfs/smb-share:server=10.8.11.190,share=test");
    TestHelper::runInLoop([ = ]() {
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, true));
    });
    urlvideos.setUrl("dfmroot:///%252Frun%252Fuser%252F1000%252Fgvfs%252Fftp%253Ahost%253D10.8.70.116.gvfsmp");
    Stub st;
    bool (*isHostAndPortConnect)(const QString &, const QString &) = [](const QString &, const QString &) {return false;};
    st.set(ADDR(CheckNetwork, isHostAndPortConnect), isHostAndPortConnect);
    TestHelper::runInLoop([ = ]() {
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, true));
    });
    TestHelper::runInLoop([ = ]() {
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, "ftp:host=10.8.70.110", true));
    });
    EXPECT_FALSE(service->checkGvfsMountfileBusy(DUrl(), "ftp:host=10.8.70.110", true));
    EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, "", true));
    TestHelper::runInLoop([ = ]() {
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, "ftp:host=10.8.70.110", true));
    });
    TestHelper::runInLoop([ = ]() {
        EXPECT_TRUE(service->checkGvfsMountfileBusy(urlvideos, "ftp:host=10.8.70.110,21", true));
    });
    ASSERT_NO_FATAL_FAILURE(service->setDoClearTrashState(true));
    EXPECT_TRUE(service->getDoClearTrashState());
    ASSERT_NO_FATAL_FAILURE(service->setDoClearTrashState(false));
    EXPECT_FALSE(service->getDoClearTrashState());

    auto ecenttmp = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction,
                                                     DUrl::fromLocalFile(to.toLocalFile()), DUrlList() << to);
    ecenttmp->setData(QVariant::fromValue(DUrlList() << to));
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp, DUrlList() << to));
    ecenttmp = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction,
                                                DUrl::fromLocalFile(to.toLocalFile()), DUrlList() << to);
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp, DUrlList() << to));
    int (*executelamda)(const QString &) = [](const QString &) {return 0;};
    stl.set((int (*)(const QString &))ADDR(QProcess, execute), executelamda);
    to.setPath("~/.cache/deepin/discburn/_dev_sr1/tesatj");
    ASSERT_NO_FATAL_FAILURE(service->dealPasteEnd(ecenttmp, DUrlList() << to));

    EXPECT_FALSE(service->isSmbFtpContain(to));
    TestHelper::runInLoop([ = ]() {
        ASSERT_NO_FATAL_FAILURE(service->onTagEditorChanged(QStringList() << "tag_ut_test", DUrlList() << to));
    });
    TestHelper::runInLoop([ = ] {
        EXPECT_TRUE(service->removeTagsOfFile(nullptr, to, QStringList() << "ut_tag_test"));
    });*/
}

TEST_F(DFileSeviceTest, start_laterRequestSelectFiles)
{
    ASSERT_NO_FATAL_FAILURE(service->laterRequestSelectFiles(DFMUrlListBaseEvent(nullptr, DUrlList())));
}

TEST_F(DFileSeviceTest, start_insertToCreatorHash)
{
    QUrl qurl;
    ASSERT_NO_FATAL_FAILURE(service->insertToCreatorHash(HandlerType(qurl.scheme(), qurl.host()),
    HandlerCreatorType(typeid(DFMFileControllerFactory).name(), [] {
        return DFMFileControllerFactory::create("key");
    })));
}

TEST_F(DFileSeviceTest, start_getSymlinkFileName)
{
    EXPECT_TRUE(service->getSymlinkFileName(DUrl()).isEmpty());
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath(TestHelper::createTmpFile());
    EXPECT_NO_FATAL_FAILURE(service->getSymlinkFileName(url).isEmpty());
    QStringList deletefilepath;
    deletefilepath << url.toLocalFile();
    url.setPath(TestHelper::createTmpFile(".txt"));
    EXPECT_NO_FATAL_FAILURE(service->getSymlinkFileName(url).isEmpty());
    deletefilepath << url.toLocalFile();
    url.setPath(TestHelper::createTmpDir());
    EXPECT_NO_FATAL_FAILURE(service->getSymlinkFileName(url).isEmpty());
    QString(*pathlamda)(void *) = [](void *) {return QString();};
    stl.set(ADDR(QDir, path), pathlamda);
    EXPECT_NO_FATAL_FAILURE(service->getSymlinkFileName(url).isEmpty());
    deletefilepath << url.toLocalFile();
    url.setPath("/bin");
    EXPECT_NO_FATAL_FAILURE(service->getSymlinkFileName(url).isEmpty());
    TestHelper::deleteTmpFiles(deletefilepath);
}

TEST_F(DFileSeviceTest, start_checkMultiSelectionFilesCache)
{
    EXPECT_FALSE(service->checkMultiSelectionFilesCache());
}
