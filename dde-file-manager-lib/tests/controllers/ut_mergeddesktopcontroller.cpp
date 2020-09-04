#include <QProcess>

#include <gtest/gtest.h>
#include "controllers/mergeddesktopcontroller.h"
#include "models/desktopfileinfo.h"
#include "dfmevent.h"
#include "interfaces/dfmstandardpaths.h"
#include "interfaces/private/mergeddesktop_common_p.h"

namespace {
class TestMergedDesktopController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMergedDesktopController";
        ctrl = new MergedDesktopController;

        filePath = DFMStandardPaths::location(DFMStandardPaths::HomePath) + "/test.files";
        QProcess::execute("touch", {filePath});
    }

    void TearDown() override
    {
        std::cout << "end TestMergedDesktopController";
        delete ctrl;
        ctrl = nullptr;

        QProcess::execute("rm", {filePath});
    }

public:
    MergedDesktopController *ctrl = nullptr;
    QString filePath;
};
} // namespace

TEST_F(TestMergedDesktopController, createFileInfo)
{
    auto e = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DesktopFileInfo::computerDesktopFileUrl());
    auto f = ctrl->createFileInfo(e);
    ASSERT_TRUE(f != nullptr);
    EXPECT_STREQ(DesktopFileInfo::computerDesktopFileUrl().path().toStdString().c_str(),
                 f->absoluteFilePath().toStdString().c_str());
}

TEST_F(TestMergedDesktopController, getChildren)
{
    auto e = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DesktopFileInfo::computerDesktopFileUrl(), QStringList(), QDir::AllEntries);
    auto l = ctrl->getChildren(e);
    EXPECT_TRUE(l.count() == 0);
}

TEST_F(TestMergedDesktopController, canCreateFileWatcher)
{
    auto e = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DesktopFileInfo::computerDesktopFileUrl());
    auto w = ctrl->createFileWatcher(e);
    EXPECT_TRUE(w != nullptr);
}

TEST_F(TestMergedDesktopController, canOpenFile)
{
    auto e = dMakeEventPointer<DFMOpenFileEvent>(nullptr, DesktopFileInfo::computerDesktopFileUrl());
    EXPECT_TRUE(ctrl->openFile(e));

    auto ee = dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << DesktopFileInfo::computerDesktopFileUrl());
    EXPECT_TRUE(ctrl->openFiles(ee));
}

TEST_F(TestMergedDesktopController, canOpenFileByApp)
{
    auto e = dMakeEventPointer<DFMOpenFileByAppEvent>(nullptr, "dde-file-manager", DUrl("computer:///"));
    EXPECT_FALSE(ctrl->openFileByApp(e));
}

TEST_F(TestMergedDesktopController, moveToTrash)
{
    auto e = dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList() << DUrl::fromLocalFile(filePath));
    EXPECT_TRUE(ctrl->moveToTrash(e).count() > 0);
}

TEST_F(TestMergedDesktopController, canWriteToClipboard)
{
    auto e = dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CopyAction, DUrlList() << DUrl::fromLocalFile(filePath));
    EXPECT_TRUE(ctrl->writeFilesToClipboard(e));
}

// 这里要弹一个框框，不知道怎么跳过，先屏蔽
//TEST_F(TestMergedDesktopController, pasteFiles) {
//    QProcess::execute("rm", {DFMStandardPaths::location(DFMStandardPaths::DownloadsPath) + "/test.files"});
//    auto e = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CopyAction,
//                                              DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DownloadsPath)),
//                                              DUrlList() << DUrl::fromLocalFile(filePath));
//    EXPECT_TRUE(ctrl->pasteFile(e).count() > 0);
//}

TEST_F(TestMergedDesktopController, canDeleteFiles)
{
    auto e = dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << DUrl::fromLocalFile(filePath), true, true);
    EXPECT_TRUE(ctrl->deleteFiles(e));
}

TEST_F(TestMergedDesktopController, canRenameFile)
{
    DUrl toUrl = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DownloadsPath) + "/test.files");
    QProcess::execute("rm", {toUrl.path()});
    auto e = dMakeEventPointer<DFMRenameEvent>(nullptr, DUrl::fromLocalFile(filePath), toUrl, true);
    EXPECT_TRUE(ctrl->renameFile(e));
}

TEST_F(TestMergedDesktopController, canOpenInTerminal)
{
    auto e = dMakeEventPointer<DFMOpenInTerminalEvent>(nullptr, DUrl::fromLocalFile(filePath));
    EXPECT_TRUE(ctrl->openInTerminal(e));
}

TEST_F(TestMergedDesktopController, canMakeDir)
{
    DUrl testUrl = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DocumentsPath) + "/ut_test/");
    auto e = dMakeEventPointer<DFMMkdirEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->mkdir(e));
    QProcess::execute("rm", {testUrl.path()});
}

TEST_F(TestMergedDesktopController, canTouchFile)
{
    DUrl testUrl = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::DocumentsPath) + "/ut_test_create_file.txt");
    auto e = dMakeEventPointer<DFMTouchFileEvent>(nullptr, testUrl);
    EXPECT_FALSE(ctrl->touch(e));
    QProcess::execute("rm", {testUrl.path()});
}

TEST_F(TestMergedDesktopController, canSetPermission)
{
    auto e = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, DUrl::fromLocalFile(filePath), QFileDevice::ReadUser | QFileDevice::WriteUser);
    EXPECT_TRUE(ctrl->setPermissions(e));
}

TEST_F(TestMergedDesktopController, canSetFileTag)
{
    auto e = dMakeEventPointer<DFMSetFileTagsEvent>(nullptr, DUrl::fromLocalFile(filePath), QList<QString>() << "testTag");
    EXPECT_TRUE(ctrl->setFileTags(e));
}

TEST_F(TestMergedDesktopController, getTagsFromFile)
{
    auto e = dMakeEventPointer<DFMGetTagsThroughFilesEvent>(nullptr, DUrlList() << DUrl::fromLocalFile(filePath));
    EXPECT_NE(int(0), ctrl->getTagsThroughFiles(e).count());
}

TEST_F(TestMergedDesktopController, staticFuncTest)
{
    EXPECT_STREQ("Pictures", MergedDesktopController::entryNameByEnum(DMD_PICTURE).toStdString().c_str());
    EXPECT_STREQ("Music", MergedDesktopController::entryNameByEnum(DMD_MUSIC).toStdString().c_str());
    EXPECT_STREQ("Applications", MergedDesktopController::entryNameByEnum(DMD_APPLICATION).toStdString().c_str());
    EXPECT_STREQ("Videos", MergedDesktopController::entryNameByEnum(DMD_VIDEO).toStdString().c_str());
    EXPECT_STREQ("Documents", MergedDesktopController::entryNameByEnum(DMD_DOCUMENT).toStdString().c_str());
    EXPECT_STREQ("Others", MergedDesktopController::entryNameByEnum(DMD_OTHER).toStdString().c_str());
    EXPECT_STREQ("Folders", MergedDesktopController::entryNameByEnum(DMD_FOLDER).toStdString().c_str());
    EXPECT_STREQ("Bug", MergedDesktopController::entryNameByEnum(DMD_TYPES(999)).toStdString().c_str());

    EXPECT_EQ(DMD_PICTURE, MergedDesktopController::entryTypeByName("Pictures"));
    EXPECT_EQ(DMD_MUSIC, MergedDesktopController::entryTypeByName("Music"));
    EXPECT_EQ(DMD_APPLICATION, MergedDesktopController::entryTypeByName("Applications"));
    EXPECT_EQ(DMD_VIDEO, MergedDesktopController::entryTypeByName("Videos"));
    EXPECT_EQ(DMD_DOCUMENT, MergedDesktopController::entryTypeByName("Documents"));
    EXPECT_EQ(DMD_OTHER, MergedDesktopController::entryTypeByName("Others"));
    EXPECT_EQ(DMD_OTHER, MergedDesktopController::entryTypeByName("ThisIsXust"));

    EXPECT_EQ(DUrl(DFMMD_ROOT VIRTUALFOLDER_FOLDER "/"), MergedDesktopController::getVirtualEntryPath(DMD_FOLDER));
    DUrl expect = DUrl(DFMMD_ROOT VIRTUALENTRY_FOLDER + MergedDesktopController::entryNameByEnum(DMD_PICTURE) + "/");
    EXPECT_EQ(expect, MergedDesktopController::getVirtualEntryPath(DMD_PICTURE));

    EXPECT_FALSE(MergedDesktopController::isVirtualEntryPaths(expect));
}
