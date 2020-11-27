#include <QProcess>
#include <QTimer>

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
} // namespace

TEST_F(TestMergedDesktopController, tstFuncsWithEvents)
{
    auto e1 = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(ctrl->createFileInfo(e1));

    auto e2 = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("dfmmd:///"), QStringList(), QDir::Filters());
    EXPECT_FALSE(ctrl->getChildren(e2).isEmpty());

    auto e3 = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl("dfmmd:///"));
    auto *watcher = ctrl->createFileWatcher(e3);
    EXPECT_TRUE(watcher);

    if (watcher) {
        watcher->setEnabledSubfileWatcher(DUrl("dfmmd:///"), true);
        watcher->setEnabledSubfileWatcher(DUrl("dfmmd:///"), false);
    }

    auto e4 = dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(ctrl->openFile(e4));

    auto e5 = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << DUrl("file:///"));
    EXPECT_TRUE(ctrl->openFiles(e5));

    auto e6 = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "dde-file-manager", DUrl("file:///"));
    EXPECT_TRUE(ctrl->openFileByApp(e6));

    // 会崩
//    QProcess::execute("touch", QStringList() << "/tmp/dde-file-manager-unit-test.txt");
//    auto e7 = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << DUrl("file:///tmp/dde-file-manager-unit-test.txt"), true);
//    EXPECT_TRUE(!ctrl->moveToTrash(e7).isEmpty());

    auto e8 = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << DUrl("file:///usr/bin/dde-file-manager"));
    EXPECT_TRUE(ctrl->writeFilesToClipboard(e8));

    // 估计会崩
//    pastFiles;
//    deleteFiles;

    QProcess::execute("touch", QStringList() << "/tmp/dde-file-manager-unit-test.txt");
    auto e11 = dMakeEventPointer<DFMRenameEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), DUrl("file:///tmp/dde-file-manager-unit-test_new_name.txt"), true);
    EXPECT_TRUE(ctrl->renameFile(e11));
    QProcess::execute("rm", QStringList() << "/tmp/dde-file-manager-unit-test_new_name.txt");

//    auto e12 = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << DUrl("file:///tmp/dde-file-manager-unit-test_new_name.txt"), true, true);
//    EXPECT_TRUE(ctrl->deleteFiles(e12));

    auto e13 = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, DUrl("file:///home"));
    EXPECT_TRUE(ctrl->openInTerminal(e13));

//    auto e14 = dMakeEventPointer<DFMMkdirEvent>(nullptr, DUrl("file:///tmp/dfm-test-dir"));
//    EXPECT_TRUE(ctrl->mkdir(e14));
//    QProcess::execute("rm", QStringList() << "rf" << "/tmp/dfm-test-dir");

//    auto e15 = dMakeEventPointer<DFMTouchFileEvent>(nullptr, DUrl("file:///tmp/test-create-by-touch.txt"));
//    EXPECT_TRUE(ctrl->touch(e15));

    QProcess::execute("touch", QStringList() << "/tmp/dde-file-manager-unit-test.txt");
    auto e16 = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QFileDevice::ReadUser | QFileDevice::ReadOwner);
    EXPECT_TRUE(ctrl->setPermissions(e16));
//    QProcess::execute("rm", QStringList() << "/tmp/dde-file-manager-unit-test.txt");

    auto e17 = dMakeEventPointer<DFMCompressEvent>(nullptr, DUrlList() << DUrl("file:///tmp/dde-file-manager-unit-test.txt"));
    EXPECT_TRUE(ctrl->compressFiles(e17));

//    auto e18 = dMakeEventPointer<DFMCreateSymlinkEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), DUrl("file:///home"), true);
//    EXPECT_TRUE(ctrl->createSymlink(e18));

    auto e19 = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, DUrl("file:///tmp/dde-file-manager-unit-test.txt"), QList<QString>() << "Test1" << "Test2");
    EXPECT_TRUE(ctrl->setFileTags(e19));

    auto e21 = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, QList<DUrl>() << DUrl("file:///tmp/dde-file-manager-unit-test.txt"));
    EXPECT_TRUE(2 == ctrl->getTagsThroughFiles(e21).count());

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
    EXPECT_TRUE(qApp->translate("MergedDesktopController", "Others") == MergedDesktopController::entryNameByEnum(DMD_TYPES::DMD_PICTURE));
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
    EXPECT_TRUE(MergedDesktopController::convertToRealPaths(DUrlList() << DUrl("dfmmd:///home")).count() == 0);

    EXPECT_FALSE(MergedDesktopController::isVirtualEntryPaths(DUrl("file:///home")));
}

TEST_F(TestMergedDesktopController, tstSlots)
{
    ctrl->desktopFilesCreated(DUrl("file:///home"));
    ctrl->desktopFilesRemoved(DUrl("file:///home"));
    ctrl->desktopFilesRenamed(DUrl(), DUrl());
}
