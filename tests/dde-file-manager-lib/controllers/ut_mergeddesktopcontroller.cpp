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

#include <QProcess>
#include <QTimer>
#include <QDialog>
#include "stub.h"
#include <QMutex>
#include <gtest/gtest.h>

#include "controllers/mergeddesktopcontroller.h"
#include "models/desktopfileinfo.h"
#include "dfmevent.h"
#include "interfaces/dfmstandardpaths.h"
#include "interfaces/private/mergeddesktop_common_p.h"
#include "dfiledevice.h"
#include "dfilehandler.h"
#include "dstorageinfo.h"
#include "dabstractfilewatcher.h"
#include "dfileservices.h"
#include "dfmglobal.h"
#include "stubext.h"

#define private public
#define protected public
#include "controllers/mergeddesktopcontroller_p.h"

namespace {
class TestMergedDesktopController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMergedDesktopController";
        ctrl = new MergedDesktopController;
    }

    void TearDown() override
    {
        std::cout << "end TestMergedDesktopController";
        QEventLoop loop;
        QTimer::singleShot(20, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();

        delete ctrl;
        ctrl = nullptr;
    }

public:
    MergedDesktopController *ctrl = nullptr;
};

class TestMergedDesktopWatcher: public testing::Test
{
public:
    void SetUp()
    {
        watcher = new MergedDesktopWatcher(DUrl("computer:///"), nullptr);
    }
    void TearDown()
    {
        delete watcher;
        watcher = nullptr;
    }

    MergedDesktopWatcher *watcher;
};
} // namespace

TEST_F(TestMergedDesktopController, tstFuncsWithEvents)
{
    auto e1 = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(ctrl->createFileInfo(e1));

    auto e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///"), QStringList(), QDir::Filters());
    {
        auto list = ctrl->getChildren(e2);
        EXPECT_FALSE(list.isEmpty());
        list.clear();
    }

    e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///entry/"), QStringList(), QDir::Filters());
    ctrl->getChildren(e2);
    e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///entry/test"), QStringList(), QDir::Filters());
    ctrl->getChildren(e2);
    e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///folder/"), QStringList(), QDir::Filters());
    ctrl->getChildren(e2);
    e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///mergeddesktop/"), QStringList(), QDir::Filters());
    ctrl->getChildren(e2);

    auto e3 = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl("dfmmd:///"));
    auto *watcher = ctrl->createFileWatcher(e3);
    EXPECT_TRUE(watcher);

    if (watcher) {
        watcher->setEnabledSubfileWatcher(DUrl("dfmmd:///"), true);
        watcher->setEnabledSubfileWatcher(DUrl("dfmmd:///"), false);
        delete watcher;
    }

    auto e4 = dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(ctrl->openFile(e4));

    auto e5 = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << DUrl("file:///"));
    EXPECT_TRUE(ctrl->openFiles(e5));

    auto e6 = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "dde-file-manager", DUrl("file:///"));
    EXPECT_FALSE(ctrl->openFileByApp(e6));

    // 原函数直接调用会崩
    DUrlList (*moveToTrash_stub)(void *, const QObject *, const DUrlList &) = [](void *, const QObject *, const DUrlList &) {
        return DUrlList();
    };
    Stub st;
    st.set(ADDR(DFileService, moveToTrash), moveToTrash_stub);
    QProcess::execute("touch", QStringList() << QDir::currentPath()+"/dde-file-manager-unit-test.txt");
    auto e7 = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << DUrl("file://"+QDir::currentPath()+"/dde-file-manager-unit-test.txt"), true);
    EXPECT_FALSE(!ctrl->moveToTrash(e7).isEmpty());
    st.reset(ADDR(DFileService, moveToTrash));

    auto e8 = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << DUrl("file:///usr/bin/dde-file-manager"));
    EXPECT_TRUE(ctrl->writeFilesToClipboard(e8));

    DUrlList (*pastFile_stub)(void *, const QObject *, DFMGlobal::ClipboardAction, const DUrl &, const DUrlList &)
            = [](void *, const QObject *, DFMGlobal::ClipboardAction, const DUrl &, const DUrlList &) {
        return DUrlList();
    };
    st.set(ADDR(DFileService, pasteFile), pastFile_stub);
    auto e9 = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::ClipboardAction::CutAction, DUrl(), DUrlList());
    ctrl->pasteFile(e9);
    st.reset(ADDR(DFileService, pasteFile));

    bool (*deleteFiles_stub)(void *, const QObject *, const DUrlList &, bool, bool, bool)
            = [](void *, const QObject *, const DUrlList &, bool, bool, bool) {
        return true;
    };
    st.set(ADDR(DFileService, deleteFiles), deleteFiles_stub);
    auto e10 = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList());
    ctrl->deleteFiles(e10);
    st.reset(ADDR(DFileService, deleteFiles));

    QProcess::execute("touch", QStringList() << QDir::currentPath()+"/dde-file-manager-unit-test.txt");
    auto e11 = dMakeEventPointer<DFMRenameEvent>(nullptr, DUrl("file://"+QDir::currentPath()+"/dde-file-manager-unit-test.txt"), DUrl("file://"+QDir::currentPath()+"/dde-file-manager-unit-test_new_name.txt"), true);
    EXPECT_TRUE(ctrl->renameFile(e11));
    QProcess::execute("rm", QStringList() << QDir::currentPath()+"/dde-file-manager-unit-test_new_name.txt");

    auto e13 = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, DUrl("file:///home"));
    EXPECT_TRUE(ctrl->openInTerminal(e13));

    bool (*mkdirAndTouch_stub)(void *, const QObject *, const DUrl &) = [](void *, const QObject *, const DUrl &){
        return true;
    };
    st.set(ADDR(DFileService, mkdir), mkdirAndTouch_stub);
    auto e14 = dMakeEventPointer<DFMMkdirEvent>(nullptr, DUrl("file:///tmp/dfm-test-dir"));
    EXPECT_TRUE(ctrl->mkdir(e14));
    st.reset(ADDR(DFileService, mkdir));

    st.set(ADDR(DFileService, touchFile), mkdirAndTouch_stub);
    auto e15 = dMakeEventPointer<DFMTouchFileEvent>(nullptr, DUrl("file:///tmp/test-create-by-touch.txt"));
    EXPECT_TRUE(ctrl->touch(e15));
    st.reset(ADDR(DFileService, touchFile));

    QProcess::execute("touch", QStringList() << "/tmp/dde-file-manager-unit-test.txt");
    auto e16 = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QFileDevice::ReadUser | QFileDevice::ReadOwner);
    EXPECT_TRUE(ctrl->setPermissions(e16));
//    QProcess::execute("rm", QStringList() << "/tmp/dde-file-manager-unit-test.txt");

    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), []{ return QDialog::Rejected; });
    stext.set_lamda(VADDR(DFileService, compressFiles), []() { return true; });
    auto e17 = dMakeEventPointer<DFMCompressEvent>(nullptr, DUrlList() << DUrl("file:///tmp/dde-file-manager-unit-test.txt"));
    EXPECT_TRUE(ctrl->compressFiles(e17));

    bool (*decompressFile_stub)(void *, const QObject *, const DUrlList &) = [](void *, const QObject *, const DUrlList &){
        return true;
    };
    st.set(ADDR(DFileService, decompressFile), decompressFile_stub);
    auto e17_2 = dMakeEventPointer<DFMDecompressEvent>(nullptr, DUrlList());
    ctrl->decompressFile(e17_2);
    st.reset(ADDR(DFileService, decompressFile));

    bool (*createSymlink_stub)(void *, const QObject *, const DUrl &, const DUrl &, bool) = [](void *, const QObject *, const DUrl &, const DUrl &, bool){
        return true;
    };
    bool (DFileService::*createSymlink)(const QObject *, const DUrl &, const DUrl &, bool) const = &DFileService::createSymlink;
    st.set(createSymlink, createSymlink_stub);
    auto e18 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), DUrl("file:///home"), true);
    EXPECT_TRUE(ctrl->createSymlink(e18));
    st.reset(createSymlink);

    stext.set_lamda(ADDR(DFileService, setFileTags), []{ return true; });
    auto e19 = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QList<QString>() << "Test1" << "Test2");
    EXPECT_TRUE(ctrl->setFileTags(e19));

    auto e21 = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>() << DUrl("file:///tmp/dde-file-manager-unit-test.txt"));
    EXPECT_FALSE(2 == ctrl->getTagsThroughFiles(e21).count());

    auto e20 = dMakeEventPointer<DFMRemoveTagsOfFileEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QList<QString>() << "Test1" << "Test2");
    EXPECT_TRUE(ctrl->removeTagsOfFile(e20));

    auto e22 = dMakeEventPointer<DFMUrlBaseEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"));
    auto *dev = ctrl->createFileDevice(e22);
    EXPECT_TRUE(dev);
    if (dev)
        delete dev;

    auto *handler = ctrl->createFileHandler(e22);
    EXPECT_TRUE(handler);
    if (handler)
        delete handler;

    auto *storage = ctrl->createStorageInfo(e22);
    EXPECT_TRUE(storage);
    if (storage)
        delete storage;

    auto e23 = dMakeEventPointer<DFMSetFileExtraProperties>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QVariantHash());
    EXPECT_TRUE(ctrl->setExtraProperties(e23));

    QProcess::execute("rm", QStringList() << "/tmp/dde-file-manager-unit-test.txt");
}

TEST_F(TestMergedDesktopController, tstStaticFuncs)
{
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Pictures") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_PICTURE));
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Music") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_MUSIC));
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Applications") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_APPLICATION));
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Videos") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_VIDEO));
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Documents") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_DOCUMENT));
    EXPECT_FALSE(qApp->translate("MergedDesktopController", "Others") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_PICTURE));
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Folders") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_FOLDER));

    EXPECT_TRUE(DMD_TYPES::DMD_PICTURE == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Pictures")));
    EXPECT_TRUE(DMD_TYPES::DMD_MUSIC == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Music")));
    EXPECT_TRUE(DMD_TYPES::DMD_APPLICATION == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Applications")));
    EXPECT_TRUE(DMD_TYPES::DMD_VIDEO == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Videos")));
    EXPECT_TRUE(DMD_TYPES::DMD_DOCUMENT == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Documents")));
    EXPECT_TRUE(DMD_TYPES::DMD_OTHER == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Others")));
    EXPECT_TRUE(DMD_TYPES::DMD_OTHER == MergedDesktopController::entryTypeByName(qApp->translate("MergedDesktopController", "Othersss")));

    EXPECT_TRUE(DUrl(DFMMD_ROOT VIRTUALFOLDER_FOLDER "/") == MergedDesktopController::getVirtualEntryPath(DMD_TYPES::DMD_FOLDER));
    EXPECT_TRUE(DUrl(DFMMD_ROOT VIRTUALENTRY_FOLDER + MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_OTHER) + "/")
                == MergedDesktopController::getVirtualEntryPath(DMD_TYPES::DMD_OTHER));

    EXPECT_TRUE(MergedDesktopController::convertToDFMMDPath(DUrl("file:///home")).isValid());

    EXPECT_TRUE(MergedDesktopController::convertToRealPath(DUrl("file:///home")).isValid());
    EXPECT_TRUE(MergedDesktopController::convertToRealPath(DUrl("dfmmd:///home")).isValid());
    EXPECT_FALSE(MergedDesktopController::convertToRealPaths(DUrlList() << DUrl("dfmmd:///home")).count() == 0);

    EXPECT_FALSE(MergedDesktopController::isVirtualEntryPaths(DUrl("file:///home")));
}

TEST_F(TestMergedDesktopController, tstSlots)
{
    ctrl->desktopFilesCreated(DUrl("file:///home"));
    ctrl->desktopFilesRemoved(DUrl("file:///home"));
    ctrl->desktopFilesRenamed(DUrl(), DUrl());
}

TEST_F(TestMergedDesktopWatcher, tstFuncs)
{
    watcher->onFileAttributeChanged(DUrl("file:///home"));
    watcher->onFileModified(DUrl("file:///home"));
    watcher->startWatcher();
    watcher->stopWatcher();
    watcher->d_func()->start();
    watcher->d_func()->stop();
}
