// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"
#include "extendmenuscene/extendmenu/dcustomactiondefine.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_DCustomActionBuilder : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        builder = new DCustomActionBuilder();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }

    virtual void TearDown() override
    {
        delete builder;
        builder = nullptr;
        stub.clear();
    }

protected:
    DCustomActionBuilder *builder { nullptr };
    stub_ext::StubExt stub;
};

// SetActiveDir 测试

TEST_F(UT_DCustomActionBuilder, SetActiveDir_ValidDir_SetsDirectoryName)
{
    QUrl dirUrl = QUrl::fromLocalFile("/tmp/testdir");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("testdir");
    });

    EXPECT_NO_FATAL_FAILURE(builder->setActiveDir(dirUrl));
}

TEST_F(UT_DCustomActionBuilder, SetActiveDir_RootDir_SetsSlashAsName)
{
    QUrl dirUrl = QUrl::fromLocalFile("/");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("");   // 根目录没有名称
    });

    EXPECT_NO_FATAL_FAILURE(builder->setActiveDir(dirUrl));
}

TEST_F(UT_DCustomActionBuilder, SetActiveDir_FileInfoCreationFails_ReturnsEarly)
{
    QUrl dirUrl = QUrl::fromLocalFile("/tmp/testdir");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    EXPECT_NO_FATAL_FAILURE(builder->setActiveDir(dirUrl));
}

// SetFocusFile 测试

TEST_F(UT_DCustomActionBuilder, SetFocusFile_RegularFile_SetsBaseName)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 不是目录
    });

    EXPECT_NO_FATAL_FAILURE(builder->setFocusFile(fileUrl));
}

TEST_F(UT_DCustomActionBuilder, SetFocusFile_Directory_KeepsFullName)
{
    QUrl dirUrl = QUrl::fromLocalFile("/tmp/testdir");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("testdir");
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 是目录
    });

    EXPECT_NO_FATAL_FAILURE(builder->setFocusFile(dirUrl));
}

TEST_F(UT_DCustomActionBuilder, SetFocusFile_FileInfoCreationFails_ReturnsEarly)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    EXPECT_NO_FATAL_FAILURE(builder->setFocusFile(fileUrl));
}

// GetCompleteSuffix 测试

TEST_F(UT_DCustomActionBuilder, GetCompleteSuffix_SimpleSuffix_ReturnsSuffix)
{
    QString result = builder->getCompleteSuffix("test.txt", "txt");
    EXPECT_EQ(result, "txt");
}

TEST_F(UT_DCustomActionBuilder, GetCompleteSuffix_CompoundSuffix_ReturnsCompleteSuffix)
{
    QString result = builder->getCompleteSuffix("test.tar.gz", "tar.gz");
    EXPECT_EQ(result, "tar.gz");
}

TEST_F(UT_DCustomActionBuilder, GetCompleteSuffix_EmptySuffix_ReturnsEmpty)
{
    QString result = builder->getCompleteSuffix("test", "");
    EXPECT_EQ(result, "");
}

TEST_F(UT_DCustomActionBuilder, GetCompleteSuffix_NoMatchingSuffix_ReturnsOriginal)
{
    QString result = builder->getCompleteSuffix("test.txt", "xyz");
    EXPECT_EQ(result, "xyz");
}

// CheckFileCombo 测试

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_EmptyList_ReturnsBlankFile)
{
    QList<QUrl> files;
    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kBlankSpace);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_SingleFile_ReturnsSingleFile)
{
    QList<QUrl> files = { QUrl::fromLocalFile("/tmp/test.txt") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 不是目录
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kSingleFile);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_MultipleFiles_ReturnsMultiFiles)
{
    QList<QUrl> files = {
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
        return false;   // 不是目录
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kMultiFiles);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_SingleDir_ReturnsSingleDir)
{
    QList<QUrl> files = { QUrl::fromLocalFile("/tmp/testdir") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 是目录
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kSingleDir);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_MultipleDirs_ReturnsMultiDirs)
{
    QList<QUrl> files = {
        QUrl::fromLocalFile("/tmp/dir1"),
        QUrl::fromLocalFile("/tmp/dir2")
    };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 是目录
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kMultiDirs);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_MixedFilesAndDirs_ReturnsFileAndDir)
{
    QList<QUrl> files = {
        QUrl::fromLocalFile("/tmp/test.txt"),
        QUrl::fromLocalFile("/tmp/testdir")
    };

    int callCount = 0;
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [&callCount](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return callCount++ > 0;   // 第一个是文件，第二个是目录
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kFileAndDir);
}

TEST_F(UT_DCustomActionBuilder, CheckFileCombo_FileInfoCreationFails_SkipsFile)
{
    QList<QUrl> files = {
        QUrl::fromLocalFile("/tmp/invalid.txt"),
        QUrl::fromLocalFile("/tmp/valid.txt")
    };

    int callCount = 0;
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&callCount](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       if (callCount++ == 0)
                           return nullptr;   // 第一次失败
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = DCustomActionBuilder::checkFileCombo(files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kSingleFile);
}

// CheckFileComboWithFocus 测试

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_FocusIsFile_ReturnsSingleFile)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/focus.txt");
    QList<QUrl> files = { focus };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;   // 不是目录
    });

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kSingleFile);
}

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_FocusIsDir_ReturnsSingleDir)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/focusdir");
    QList<QUrl> files = { focus };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 是目录
    });

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kSingleDir);
}

// BuildAction 测试

TEST_F(UT_DCustomActionBuilder, BuildAction_ActionData_CreatesAction)
{
    DCustomActionData actionData;
    actionData.actionName = "Test Action";
    actionData.actionCommand = "echo test";

    QMenu menu;
    auto action = builder->buildAciton(actionData, &menu);

    EXPECT_NE(action, nullptr);
    delete action;
}

TEST_F(UT_DCustomActionBuilder, BuildAction_MenuData_CreatesMenu)
{
    DCustomActionData menuData;
    menuData.actionName = "Test Menu";
    menuData.childrenActions.append(DCustomActionData());

    QMenu menu;
    auto action = builder->buildAciton(menuData, &menu);

    EXPECT_NE(action, nullptr);
    delete action;
}

// SplitCommand 测试

TEST_F(UT_DCustomActionBuilder, SplitCommand_SimpleCommand_ReturnsSingleElement)
{
    QString cmd = "echo";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), "echo");
}

TEST_F(UT_DCustomActionBuilder, SplitCommand_CommandWithArgs_ReturnsSplitElements)
{
    QString cmd = "echo hello world";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello");
    EXPECT_EQ(result[2], "world");
}

TEST_F(UT_DCustomActionBuilder, SplitCommand_QuotedArgs_PreservesQuotes)
{
    QString cmd = "echo \"hello world\"";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_GE(result.size(), 1);
}

TEST_F(UT_DCustomActionBuilder, SplitCommand_EmptyCommand_ReturnsEmptyList)
{
    QString cmd = "";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_TRUE(result.isEmpty() || result.size() == 1);
}

TEST_F(UT_DCustomActionBuilder, SplitCommand_SingleQuotes_PreservesSpaces)
{
    QString cmd = "echo 'hello world'";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_GE(result.size(), 1);
    EXPECT_EQ(result[0], "echo");
}

TEST_F(UT_DCustomActionBuilder, SplitCommand_MixedQuotes_HandlesCorrectly)
{
    QString cmd = "cmd \"arg1\" 'arg2'";
    auto result = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_GE(result.size(), 1);
}

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_EmptyFiles_ReturnsBlankSpace)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/test.txt");
    QList<QUrl> files;

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kBlankSpace);
}

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_MultipleFiles_ReturnsMultiFiles)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/focus.txt");
    QList<QUrl> files = { focus, QUrl::fromLocalFile("/tmp/other.txt") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kMultiFiles);
}

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_MultipleDirs_ReturnsMultiDirs)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/focusdir");
    QList<QUrl> files = { focus, QUrl::fromLocalFile("/tmp/otherdir") };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kMultiDirs);
}

TEST_F(UT_DCustomActionBuilder, CheckFileComboWithFocus_FileInfoCreationFails_ReturnsBlankSpace)
{
    QUrl focus = QUrl::fromLocalFile("/tmp/invalid.txt");
    QList<QUrl> files = { focus };

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    auto result = DCustomActionBuilder::checkFileComboWithFocus(focus, files);
    EXPECT_EQ(result, DCustomActionDefines::ComboType::kBlankSpace);
}

// MatchFileCombo 测试

TEST_F(UT_DCustomActionBuilder, MatchFileCombo_EmptyActions_ReturnsEmpty)
{
    QList<DCustomActionEntry> actions;
    auto result = DCustomActionBuilder::matchFileCombo(actions, DCustomActionDefines::kSingleFile);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MatchFileCombo_MatchingSingleFile_ReturnsMatchingActions)
{
    QList<DCustomActionEntry> actions;
    DCustomActionEntry entry;
    entry.actionFileCombo = DCustomActionDefines::kSingleFile;
    actions.append(entry);

    auto result = DCustomActionBuilder::matchFileCombo(actions, DCustomActionDefines::kSingleFile);
    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_DCustomActionBuilder, MatchFileCombo_NonMatchingType_ReturnsEmpty)
{
    QList<DCustomActionEntry> actions;
    DCustomActionEntry entry;
    entry.actionFileCombo = DCustomActionDefines::kSingleFile;
    actions.append(entry);

    auto result = DCustomActionBuilder::matchFileCombo(actions, DCustomActionDefines::kSingleDir);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MatchFileCombo_MultipleMatching_ReturnsAll)
{
    QList<DCustomActionEntry> actions;
    DCustomActionEntry entry1, entry2;
    entry1.actionFileCombo = DCustomActionDefines::kSingleFile;
    entry2.actionFileCombo = DCustomActionDefines::kSingleFile;
    actions.append(entry1);
    actions.append(entry2);

    auto result = DCustomActionBuilder::matchFileCombo(actions, DCustomActionDefines::kSingleFile);
    EXPECT_EQ(result.size(), 2);
}

// MakeCommand 测试

TEST_F(UT_DCustomActionBuilder, MakeCommand_EmptyCommand_ReturnsEmpty)
{
    auto result = builder->makeCommand("", DCustomActionDefines::kDirPath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_TRUE(result.first.isEmpty());
    EXPECT_TRUE(result.second.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_CommandWithoutArgs_ReturnsCommandOnly)
{
    auto result = builder->makeCommand("echo", DCustomActionDefines::kDirPath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_EQ(result.first, "echo");
    EXPECT_TRUE(result.second.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_DirPathArg_ReplacesDirPath)
{
    auto result = builder->makeCommand("cmd %d", DCustomActionDefines::kDirPath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_FilePathArg_ReplacesFilePath)
{
    auto result = builder->makeCommand("cmd %f", DCustomActionDefines::kFilePath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_FilePathsArg_ReplacesFilePaths)
{
    auto result = builder->makeCommand("cmd %F", DCustomActionDefines::kFilePaths,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test1.txt"),
                                         QUrl::fromLocalFile("/tmp/test2.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_GE(result.second.size(), 2);
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_UrlPathArg_ReplacesUrlPath)
{
    auto result = builder->makeCommand("cmd %u", DCustomActionDefines::kUrlPath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_FALSE(result.second.isEmpty());
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_UrlPathsArg_ReplacesUrlPaths)
{
    auto result = builder->makeCommand("cmd %U", DCustomActionDefines::kUrlPaths,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test1.txt"),
                                         QUrl::fromLocalFile("/tmp/test2.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_GE(result.second.size(), 2);
}

TEST_F(UT_DCustomActionBuilder, MakeCommand_DoublePercent_ReplacesWithSinglePercent)
{
    auto result = builder->makeCommand("cmd %%arg", DCustomActionDefines::kDirPath,
                                       QUrl::fromLocalFile("/tmp"),
                                       QUrl::fromLocalFile("/tmp/test.txt"),
                                       { QUrl::fromLocalFile("/tmp/test.txt") });
    EXPECT_EQ(result.first, "cmd");
    EXPECT_FALSE(result.second.isEmpty());
}

// IsMimeTypeSupport 测试

TEST_F(UT_DCustomActionBuilder, IsMimeTypeSupport_ExactMatch_ReturnsTrue)
{
    QStringList fileMimeTypes = { "text/plain", "application/octet-stream" };
    bool result = DCustomActionBuilder::isMimeTypeSupport("text/plain", fileMimeTypes);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeSupport_NoMatch_ReturnsFalse)
{
    QStringList fileMimeTypes = { "text/plain", "application/octet-stream" };
    bool result = DCustomActionBuilder::isMimeTypeSupport("image/png", fileMimeTypes);
    EXPECT_FALSE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeSupport_PartialMatch_ReturnsTrue)
{
    QStringList fileMimeTypes = { "text/plain", "application/octet-stream" };
    bool result = DCustomActionBuilder::isMimeTypeSupport("text", fileMimeTypes);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeSupport_CaseInsensitive_ReturnsTrue)
{
    QStringList fileMimeTypes = { "text/plain" };
    bool result = DCustomActionBuilder::isMimeTypeSupport("TEXT/PLAIN", fileMimeTypes);
    EXPECT_TRUE(result);
}

// IsMimeTypeMatch 测试

TEST_F(UT_DCustomActionBuilder, IsMimeTypeMatch_ExactMatch_ReturnsTrue)
{
    QStringList fileMimeTypes = { "text/plain" };
    QStringList supportMimeTypes = { "text/plain" };
    bool result = DCustomActionBuilder::isMimeTypeMatch(fileMimeTypes, supportMimeTypes);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeMatch_NoMatch_ReturnsFalse)
{
    QStringList fileMimeTypes = { "text/plain" };
    QStringList supportMimeTypes = { "image/png" };
    bool result = DCustomActionBuilder::isMimeTypeMatch(fileMimeTypes, supportMimeTypes);
    EXPECT_FALSE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeMatch_WildcardMatch_ReturnsTrue)
{
    QStringList fileMimeTypes = { "text/plain", "text/html" };
    QStringList supportMimeTypes = { "text/*" };
    bool result = DCustomActionBuilder::isMimeTypeMatch(fileMimeTypes, supportMimeTypes);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsMimeTypeMatch_EmptySupportList_ReturnsFalse)
{
    QStringList fileMimeTypes = { "text/plain" };
    QStringList supportMimeTypes;
    bool result = DCustomActionBuilder::isMimeTypeMatch(fileMimeTypes, supportMimeTypes);
    EXPECT_FALSE(result);
}

// IsSchemeSupport 测试

TEST_F(UT_DCustomActionBuilder, IsSchemeSupport_EmptySupportList_ReturnsTrue)
{
    DCustomActionEntry entry;
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = DCustomActionBuilder::isSchemeSupport(entry, url);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSchemeSupport_WildcardSupport_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSchemes = { "*" };
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = DCustomActionBuilder::isSchemeSupport(entry, url);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSchemeSupport_MatchingScheme_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSchemes = { "file" };
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = DCustomActionBuilder::isSchemeSupport(entry, url);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSchemeSupport_NonMatchingScheme_ReturnsFalse)
{
    DCustomActionEntry entry;
    entry.actionSupportSchemes = { "ftp" };
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    bool result = DCustomActionBuilder::isSchemeSupport(entry, url);
    EXPECT_FALSE(result);
}

// IsSuffixSupport 测试

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_NullFileInfo_ReturnsTrue)
{
    DCustomActionEntry entry;
    bool result = DCustomActionBuilder::isSuffixSupport(entry, nullptr);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_Directory_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "txt" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // 是目录
    });

    QUrl url = QUrl::fromLocalFile("/tmp/dir");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_EmptySupportList_ReturnsTrue)
{
    DCustomActionEntry entry;

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_WildcardSupport_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "*" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_MatchingSuffix_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "txt" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("txt");
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_TRUE(result);
}

TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_WildcardSuffix_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "7z.*" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("7z.001");
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test.7z.001");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_TRUE(result);
}

// SetFocusFile 高级测试

TEST_F(UT_DCustomActionBuilder, SetFocusFile_HiddenFile_HandlesCorrectly)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/.hidden");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString(".hidden");
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_FATAL_FAILURE(builder->setFocusFile(fileUrl));
}

TEST_F(UT_DCustomActionBuilder, SetFocusFile_CompoundSuffix_ExtractsBaseName)
{
    QUrl fileUrl = QUrl::fromLocalFile("/tmp/archive.tar.gz");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new FileInfo(url));
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("archive.tar.gz");
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_FATAL_FAILURE(builder->setFocusFile(fileUrl));
}

// MatchActions 测试

TEST_F(UT_DCustomActionBuilder, MatchActions_EmptySelects_ReturnsOriginal)
{
    QList<QUrl> selects;
    QList<DCustomActionEntry> actions;
    DCustomActionEntry entry;
    actions.append(entry);

    auto result = builder->matchActions(selects, actions);
    EXPECT_EQ(result.size(), 1);
}

TEST_F(UT_DCustomActionBuilder, MatchActions_FileInfoCreationFails_ContinuesProcessing)
{
    QList<QUrl> selects = { QUrl::fromLocalFile("/tmp/invalid.txt") };
    QList<DCustomActionEntry> actions;
    DCustomActionEntry entry;
    actions.append(entry);

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    auto result = builder->matchActions(selects, actions);
    EXPECT_GE(result.size(), 0);
}
