/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include <QTimer>
#include <QDir>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <QDialog>

#include "dfmevent.h"
#include "interfaces/dfmstandardpaths.h"
#include "stub.h"
#include "stubext.h"
#include "testhelper.h"
#include "addr_pri.h"
#include "disomaster.h"
#include "dfileservices.h"

#define private public
#define protected public
#include "controllers/masteredmediacontroller.h"
#include "controllers/masteredmediacontroller_p.h"

namespace {

QByteArrayList (*mountPoints_stub)(void *) = [](void *) {
    return QByteArrayList() << QByteArray("test///");
};
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
        TestHelper::runInLoop([](){},
        500);
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

    Stub st;
    DUrlList (*pastFile_stub)(void *, const QObject *, DFMGlobal::ClipboardAction , const DUrl &, const DUrlList &) = [](void *, const QObject *, DFMGlobal::ClipboardAction , const DUrl &, const DUrlList &){
        return DUrlList();
    };
    st.set(&DFileService::pasteFile, pastFile_stub);

    auto e5 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, testUrl, DUrlList() << testUrl);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());
    e5 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, testUrl, DUrlList() << testUrl << testUrl);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());
    st.set(ADDR(DBlockDevice, mountPoints), mountPoints_stub);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());
    DISOMasterNS::DeviceProperty (*getDevicePropertyCached_stub)(void *, QString) = [](void *, QString) {
        DISOMasterNS::DeviceProperty dp;
        dp.devid = "testId";
        return dp;
    };
    st.set(&DISOMasterNS::DISOMaster::getDevicePropertyCached, getDevicePropertyCached_stub);
    e5 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction, testUrl, DUrlList() << testUrl);
    EXPECT_EQ(int(0), ctrl->pasteFile(e5).count());

    auto e6 = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << testUrl, true, true);
    EXPECT_FALSE(ctrl->deleteFiles(e6));

    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), []{ return 1; }); // 避免某些机器没有 file-roller 而弹框阻塞ut运行

    bool (*stub_compressFiles)()=[](){return false;};
    st.set(ADDR(DFileService, compressFiles), stub_compressFiles);
    auto e7 = dMakeEventPointer<DFMCompressEvent>(nullptr, DUrlList() << testUrl);
    EXPECT_FALSE(ctrl->compressFiles(e7));

    bool (*stub_decompressFile)()=[](){return false;};
    st.set(ADDR(DFileService, decompressFile), stub_decompressFile);
    auto e8 = dMakeEventPointer<DFMDecompressEvent>(nullptr, DUrlList() << testUrl);
    EXPECT_FALSE(ctrl->decompressFile(e8));

    auto e9 = dMakeEventPointer<DFMFileShareEvent>(nullptr, testUrl, "test");
    EXPECT_FALSE(ctrl->shareFolder(e9));

    bool (*stub_unShareFolder)()=[](){return false;};
    st.set(ADDR(DFileService, unShareFolder), stub_unShareFolder);
    auto e10 = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->unShareFolder(e10));

    bool (*stub_burnIsOnDisc)()=[](){
        return false;
    };
    st.set(&DUrl::burnIsOnDisc, stub_burnIsOnDisc);
    auto e11 = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->openInTerminal(e11));

    auto e12 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DownloadsPath)), testUrl);
    EXPECT_FALSE(ctrl->createSymlink(e12));
    e12 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, testUrl, testUrl);
    EXPECT_FALSE(ctrl->createSymlink(e12));

    stext.set_lamda(VADDR(DFileService, touchFile), [](){ return true; });
    auto e13 = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, testUrl);
    EXPECT_TRUE(ctrl->addToBookmark(e13));

    stext.set_lamda(VADDR(DFileService, deleteFiles), [](){ return true; });
    auto e14 = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, testUrl);
    EXPECT_TRUE(ctrl->removeBookmark(e14));

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
        // std::cout << "start TestMasteredMediaFileWatcher";
        // watcher = new MasteredMediaFileWatcher(DUrl("burn:///dev/sr0/disc_files/test.file"));
    }

    void TearDown() override
    {
        // std::cout << "end TestMasteredMediaFileWatcher";
        // TestHelper::runInLoop([](){},
        // 500);
        // delete watcher;
        // watcher = nullptr;
    }

    MasteredMediaFileWatcher *watcher = nullptr;
};
}

TEST_F(TestMasteredMediaFileWatcher, tstSlots)
{
//    call_private_fun::MasteredMediaFileWatcheronFileMoved(*watcher, DUrl(), DUrl());
//    call_private_fun::MasteredMediaFileWatcheronFileDeleted(*watcher, DUrl());
//    call_private_fun::MasteredMediaFileWatcheronFileAttributeChanged(*watcher, DUrl());
//    call_private_fun::MasteredMediaFileWatcheronSubfileCreated(*watcher, DUrl());

//    MasteredMediaFileWatcherPrivate *pri = new MasteredMediaFileWatcherPrivate(watcher);
//    pri->start();
//    pri->stop();
//    pri->handleGhostSignal(DUrl("burn:///dev/sr0/disc_files/test.file"), DAbstractFileWatcher::SignalType1(), DUrl());
//    pri->handleGhostSignal(DUrl("/fake/news/"), DAbstractFileWatcher::SignalType1(), DUrl());
}

namespace  {
class TestDFMShadowedDirIterator: public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMShadowedDirIterator";
        // iter = new DFMShadowedDirIterator(QUrl("burn:///dev/sr0/disc_files/test.file"), QStringList(), QDir::Filters(), QDirIterator::IteratorFlags());
    }
    void TearDown() override
    {
        std::cout << "end TestDFMShadowedDirIterator";
        // delete iter;
        // iter = nullptr;
    }

    DFMShadowedDirIterator *iter = nullptr;
};
}

TEST_F(TestDFMShadowedDirIterator, tstFuncs)
{
    // EXPECT_FALSE(!iter->next().isValid());
    // EXPECT_FALSE(iter->hasNext());
    // EXPECT_TRUE(iter->fileName().isEmpty());
    // EXPECT_FALSE(!iter->fileUrl().isValid());
    // EXPECT_FALSE(iter->fileInfo());
    // EXPECT_TRUE(!iter->url().isVaultFile());


    bool (*opticalBlank_stub)(void *) = [](void *) {
        return true;
    };
    QString (*drive_stub)(void *) = [](void *) {
        return QString("fakedev");
    };
    Stub st;
    st.set(ADDR(DBlockDevice, drive), drive_stub);
    st.set(ADDR(DBlockDevice, mountPoints), mountPoints_stub);
    st.set(ADDR(DDiskDevice, opticalBlank), opticalBlank_stub);
    // MasteredMediaFileWatcher *watcher = new MasteredMediaFileWatcher(DUrl("burn:///dev/sr0/disc_files/test.file"));
    // DFMShadowedDirIterator *tmp = new DFMShadowedDirIterator(QUrl("burn:///dev/sr0/disc_files/test.file"), QStringList(),  QDir::Filters(), QDirIterator::IteratorFlags());
    // delete tmp;
    st.reset(ADDR(DBlockDevice, mountPoints));
    st.reset(ADDR(DDiskDevice, opticalBlank));

    // watcher->d_func()->proxyOnDisk->fileDeleted(DUrl("/fake/home"));
    // watcher->d_func()->diskm.data()->opticalChanged("fakedev");
}
