#include <gtest/gtest.h>
#include <QTimer>
#include <QDir>


#include "controllers/masteredmediacontroller.h"
#include "dfmevent.h"
#include "interfaces/dfmstandardpaths.h"
#include "stub.h"
#include "addr_pri.h"

namespace {
class TestMasteredMediaController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMasteredMediaController";
        ctrl = new MasteredMediaController;
        testUrl = DUrl("burn:///dev/sr0/disc_files/test.file");
    }

    void TearDown() override
    {
        std::cout << "end TestMasteredMediaController";
        QEventLoop loop;
        QTimer::singleShot(20, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete ctrl;
        ctrl = nullptr;
    }

public:
    MasteredMediaController *ctrl = nullptr;
    DUrl testUrl;
};
} // namespace

// 这部分依赖光驱，会崩
TEST_F(TestMasteredMediaController, tstEventsFuncs)
{
    auto e = dMakeEventPointer<DFMOpenFileEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->openFile(e));

    auto e1 = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "dde-file-manager", testUrl);
    EXPECT_FALSE(ctrl->openFileByApp(e1));

    auto e2 = dMakeEventPointer<DFMOpenFilesByAppEvent>(nullptr, "dde-file-manager", DUrlList() << testUrl);
    EXPECT_FALSE(ctrl->openFilesByApp(e2));

    auto e3 = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << testUrl);
    EXPECT_EQ(int(0), ctrl->moveToTrash(e3).count());

    auto e4 = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << testUrl);
    EXPECT_TRUE(ctrl->writeFilesToClipboard(e4));

    auto e5 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, testUrl, DUrlList() << testUrl);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());
    e5 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, testUrl, DUrlList() << testUrl << testUrl);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());

    auto e6 = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << testUrl, true, true);
    EXPECT_FALSE(ctrl->deleteFiles(e6));

    auto e7 = dMakeEventPointer<DFMCompressEvent>(nullptr, DUrlList() << testUrl);
    EXPECT_FALSE(ctrl->compressFiles(e7));

    auto e8 = dMakeEventPointer<DFMDecompressEvent>(nullptr, DUrlList() << testUrl);
    EXPECT_FALSE(ctrl->decompressFile(e8));

    auto e9 = dMakeEventPointer<DFMFileShareEvent>(nullptr, testUrl, "test");
    EXPECT_FALSE(ctrl->shareFolder(e9));

    auto e10 = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->unShareFolder(e10));

    auto e11 = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->openInTerminal(e11));

    auto e12 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DownloadsPath)), testUrl);
    EXPECT_FALSE(ctrl->createSymlink(e12));
    e12 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, testUrl, testUrl);
    EXPECT_FALSE(ctrl->createSymlink(e12));

    auto e13 = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->addToBookmark(e13));

    auto e14 = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->removeBookmark(e14));

    auto e15 = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, testUrl);
    EXPECT_TRUE(ctrl->createFileInfo(e15));

    auto e16 = dMakeEventPointer<DFMCreateDiriterator>(nullptr, testUrl, QStringList(), QDir::AllEntries);
    EXPECT_TRUE(ctrl->createDirIterator(e16));

    auto e17 = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, testUrl);
    EXPECT_TRUE(ctrl->createFileWatcher(e17));
}

TEST_F(TestMasteredMediaController, tstGetPermissionsCopyToLocal)
{
    EXPECT_TRUE((QFileDevice::WriteUser | QFileDevice::ReadUser
                 | QFileDevice::WriteGroup | QFileDevice::ReadGroup
                 | QFileDevice::ReadOther) == MasteredMediaController::getPermissionsCopyToLocal());
}

ACCESS_PRIVATE_FUN(MasteredMediaFileWatcher, void(const DUrl &), onFileDeleted);
ACCESS_PRIVATE_FUN(MasteredMediaFileWatcher, void(const DUrl &), onFileAttributeChanged);
ACCESS_PRIVATE_FUN(MasteredMediaFileWatcher, void(const DUrl &, const DUrl &), onFileMoved);
ACCESS_PRIVATE_FUN(MasteredMediaFileWatcher, void(const DUrl &), onSubfileCreated);

namespace  {
class TestMasteredMediaFileWatcher: public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMasteredMediaFileWatcher";
        watcher = new MasteredMediaFileWatcher(DUrl("burn:///dev/sr0/disc_files/test.file"));
    }

    void TearDown() override
    {
        std::cout << "end TestMasteredMediaFileWatcher";
        QEventLoop loop;
        QTimer::singleShot(20, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete watcher;
        watcher = nullptr;
    }

    MasteredMediaFileWatcher *watcher = nullptr;
};
}

TEST_F(TestMasteredMediaFileWatcher, tstSlots)
{
    call_private_fun::MasteredMediaFileWatcheronFileMoved(*watcher, DUrl(), DUrl());
    call_private_fun::MasteredMediaFileWatcheronFileDeleted(*watcher, DUrl());
    call_private_fun::MasteredMediaFileWatcheronFileAttributeChanged(*watcher, DUrl());
    call_private_fun::MasteredMediaFileWatcheronSubfileCreated(*watcher, DUrl());
}

namespace  {
class TestDFMShadowedDirIterator: public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMShadowedDirIterator";
        iter = new DFMShadowedDirIterator(QUrl("burn:///dev/sr0/disc_files/test.file"), QStringList(), QDir::Filters(), QDirIterator::IteratorFlags());
    }
    void TearDown() override
    {
        std::cout << "end TestDFMShadowedDirIterator";
        delete iter;
        iter = nullptr;
    }

    DFMShadowedDirIterator *iter = nullptr;
};
}

TEST_F(TestDFMShadowedDirIterator, tstFuncs)
{
    EXPECT_TRUE(!iter->next().isValid());
    EXPECT_FALSE(iter->hasNext());
    EXPECT_TRUE(iter->fileName().isEmpty());
    EXPECT_TRUE(!iter->fileUrl().isValid());
    EXPECT_TRUE(!iter->fileInfo());
    EXPECT_TRUE(!iter->url().isVaultFile());
}
