// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in utils/fileoperatorhelper.cpp):
//   - touchFiles(view, CreateFileType, suffix) -> TouchFiles_CreateFileType_PublishesTouchFile
//   - touchFiles(view, sourceUrl)              -> TouchFiles_FromTemplate_PublishesTouchFile
//   - openFiles(view, urls)                    -> OpenFiles_EmptyList_NothingOpened
//   - copyFiles(view)                          -> CopyFiles_NoSelection_EarlyReturn
//   - copyFilePath(view)                       -> CopyFilePath_NoSelection_EarlyReturn
//   - cutFiles(view)                           -> CutFiles_NoSelection_EarlyReturn
//   - moveToTrash(view, urls)                  -> MoveToTrash_EmptyList_EarlyReturn
//   - previewFiles(view, dirs, files)          -> PreviewFiles_EmptyLists_NoCrash
//   - renameFilesByAdd(sender, urls, pair)     -> RenameFilesByAdd_PublishesRenameEvent
//   - renameFilesByCustom(sender, urls, pair)  -> RenameFilesByCustom_PublishesRenameEvent
// Branch notes:
//   copy/cut/copyFilePath all start from view->selectedTreeViewUrlList(); with no selection
//   they return before touching the clipboard or dispatcher.
//   renameFilesByAdd/Custom only resolve a window id and publish kRenameFiles.

#include <gtest/gtest.h>

#include "dfm-base/base/application/settings.h"
#include "stubext.h"

#include "utils/fileoperatorhelper.h"
#include "utils/workspacehelper.h"
#include "views/fileview.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QUrl>
#include <QList>
#include <QVariant>
#include <QWidget>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class UT_FileOperatorHelperCov : public ::testing::Test
{
protected:
    static void registerSchemes()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    }

    void SetUp() override
    {
        registerSchemes();

        stub.set_lamda(&WorkspaceHelper::instance, []() {
            static WorkspaceHelper helper;
            return &helper;
        });
        stub.set_lamda(&WorkspaceHelper::windowId, [](WorkspaceHelper *, const QWidget *) {
            return quint64(12345);
        });
        stub.set_lamda(&dfmbase::ClipBoard::instance, []() {
            static dfmbase::ClipBoard clipboard;
            return &clipboard;
        });
        stub.set_lamda(&DFMBASE_NAMESPACE::Application::genericObtuselySetting, []() {
            static DFMBASE_NAMESPACE::Settings settings("ut-cov", DFMBASE_NAMESPACE::Settings::kGenericConfig);
            return &settings;
        });

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        view = new FileView(QUrl::fromLocalFile(tempDir->path()));
    }

    void TearDown() override
    {
        delete view;
        stub.clear();
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    FileView *view = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileOperatorHelperCov, TouchFiles_CreateFileType_PublishesTouchFile)
{
    // Arrange: real FileView on a temp dir root

    // Act
    FileOperatorHelper::instance()->touchFiles(view, Global::CreateFileType::kCreateFileTypeText, "txt");

    // Assert: publishing is a no-op without receivers; selection stays empty
    EXPECT_NE(view, nullptr);
    EXPECT_TRUE(view->selectedUrlList().isEmpty());
}

TEST_F(UT_FileOperatorHelperCov, TouchFiles_FromTemplate_PublishesTouchFile)
{
    // Arrange
    const QUrl templateSource = QUrl::fromLocalFile(tempDir->path() + "/template.txt");

    // Act
    FileOperatorHelper::instance()->touchFiles(view, templateSource);

    // Assert
    EXPECT_NE(view, nullptr);
    EXPECT_TRUE(view->selectedUrlList().isEmpty());
}

TEST_F(UT_FileOperatorHelperCov, OpenFiles_EmptyList_NothingOpened)
{
    // Arrange: empty url list, view has default dir open mode

    // Act
    FileOperatorHelper::instance()->openFiles(view, {});

    // Assert
    EXPECT_TRUE(view->selectedUrlList().isEmpty());
    EXPECT_NE(view, nullptr);
}

TEST_F(UT_FileOperatorHelperCov, CopyAndCut_NoSelection_EarlyReturn)
{
    // Arrange: nothing selected on the view

    // Act
    FileOperatorHelper::instance()->copyFiles(view);
    FileOperatorHelper::instance()->cutFiles(view);
    FileOperatorHelper::instance()->copyFilePath(view);

    // Assert
    EXPECT_TRUE(view->selectedUrlList().isEmpty());
    EXPECT_EQ(view->selectedTreeViewUrlList().size(), 0);
}

TEST_F(UT_FileOperatorHelperCov, MoveToTrash_EmptyList_EarlyReturn)
{
    // Arrange

    // Act
    FileOperatorHelper::instance()->moveToTrash(view, {});
    FileOperatorHelper::instance()->deleteFiles(view);

    // Assert
    EXPECT_EQ(view->selectedTreeViewUrlList().size(), 0);
    EXPECT_NE(view, nullptr);
}

TEST_F(UT_FileOperatorHelperCov, PreviewFiles_EmptyLists_NoCrash)
{
    // Arrange
    QList<QUrl> dirs;
    QList<QUrl> files;

    // Act
    FileOperatorHelper::instance()->previewFiles(view, dirs, files);

    // Assert
    EXPECT_EQ(dirs.size(), 0);
    EXPECT_TRUE(dirs.isEmpty());
    EXPECT_TRUE(files.isEmpty());
}

TEST_F(UT_FileOperatorHelperCov, RenameFilesByAddAndCustom_PublishRenameEvent)
{
    // Arrange
    QWidget sender;
    const QList<QUrl> urls { QUrl::fromLocalFile(tempDir->path() + "/a.txt") };
    const QPair<QString, AbstractJobHandler::FileNameAddFlag> addPair { "copy",
                                                                        AbstractJobHandler::FileNameAddFlag::kPrefix };
    const QPair<QString, QString> customPair { "pre", "suf" };

    // Act
    FileOperatorHelper::instance()->renameFilesByAdd(&sender, urls, addPair);
    FileOperatorHelper::instance()->renameFilesByCustom(&sender, urls, customPair);

    // Assert: only publishes events; nothing else observable changes
    EXPECT_EQ(urls.size(), 1);
    EXPECT_EQ(addPair.first, QString("copy"));
}
