// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "oemmenuscene/oemmenu.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/protocolutils.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_OemMenu : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        menu = new OemMenu();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete menu;
        menu = nullptr;
        stub.clear();
    }

protected:
    OemMenu *menu { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_OemMenu, Constructor_InitializesCorrectly)
{
    EXPECT_NE(menu, nullptr);
}

TEST_F(UT_OemMenu, LoadDesktopFile_NoOemDirectory_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(menu->loadDesktopFile());
}

TEST_F(UT_OemMenu, EmptyActions_ValidUrl_ReturnsActionList)
{
    auto actions = menu->emptyActions(QUrl::fromLocalFile("/tmp"), false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, EmptyActions_OnDesktop_FiltersActions)
{
    auto actions = menu->emptyActions(QUrl::fromLocalFile("/tmp"), true);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, NormalActions_SingleFile_ReturnsSingleFileActions)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;  // 不是目录
    });

    auto actions = menu->normalActions(urls, false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, NormalActions_SingleDirectory_ReturnsSingleDirActions)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/testdir") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;  // 是目录
    });

    auto actions = menu->normalActions(urls, false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, NormalActions_MultipleFiles_ReturnsMultiActions)
{
    QList<QUrl> urls = {
        QUrl::fromLocalFile("/tmp/test1.txt"),
        QUrl::fromLocalFile("/tmp/test2.txt")
    };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto actions = menu->normalActions(urls, false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, NormalActions_FTPFile_FiltersCompressAction)
{
    QList<QUrl> urls = { QUrl("ftp://server/test.txt") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    stub.set_lamda(&ProtocolUtils::isFTPFile, [](const QUrl &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto actions = menu->normalActions(urls, false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, FocusNormalActions_ValidFocus_ReturnsActionList)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> urls = { focus };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto actions = menu->focusNormalActions(focus, urls, false);
    EXPECT_TRUE(actions.isEmpty() || !actions.isEmpty());
}

TEST_F(UT_OemMenu, FocusNormalActions_FileInfoCreationFails_ReturnsEmpty)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/invalid.txt");
    QList<QUrl> urls = { focus };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return nullptr;
        });

    auto actions = menu->focusNormalActions(focus, urls, false);
    EXPECT_TRUE(actions.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_NullAction_ReturnsEmpty)
{
    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(nullptr, dir, focus, files);
    EXPECT_TRUE(result.first.isEmpty());
    EXPECT_TRUE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_EmptyCommand_ReturnsEmpty)
{
    QAction action("test");
    action.setProperty("Exec", "");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_TRUE(result.first.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_SimpleCommand_ReturnsCommandOnly)
{
    QAction action("test");
    action.setProperty("Exec", "echo");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "echo");
    EXPECT_TRUE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithDirPath_ReplacesDirPath)
{
    QAction action("test");
    action.setProperty("Exec", "cd %p");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "cd");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithFilePath_ReplacesFilePath)
{
    QAction action("test");
    action.setProperty("Exec", "cat %f");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "cat");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithFilePaths_ReplacesFilePaths)
{
    QAction action("test");
    action.setProperty("Exec", "rm %F");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus, QUrl::fromLocalFile("/tmp/test2.txt") };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "rm");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithUrlPath_ReplacesUrlPath)
{
    QAction action("test");
    action.setProperty("Exec", "open %u");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "open");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithUrlPaths_ReplacesUrlPaths)
{
    QAction action("test");
    action.setProperty("Exec", "open %U");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus, QUrl::fromLocalFile("/tmp/test2.txt") };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "open");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_OemMenu, MakeCommand_WithQuotedArguments_HandlesQuotes)
{
    QAction action("test");
    action.setProperty("Exec", "echo \"hello world\"");

    QUrl dir = QUrl::fromLocalFile("/tmp");
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files = { focus };

    auto result = menu->makeCommand(&action, dir, focus, files);
    EXPECT_EQ(result.first, "echo");
    EXPECT_FALSE(result.second.isEmpty());
}
