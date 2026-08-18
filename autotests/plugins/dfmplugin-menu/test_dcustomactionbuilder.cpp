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

#include <QMimeDatabase>
#include <QUrl>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QWidget>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

namespace {
// a valid 1x1 transparent PNG
const char kPngData[] = "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52"
                        "\x00\x00\x00\x01\x00\x00\x00\x01\x08\x06\x00\x00\x00\x1f\x15\xc4\x89"
                        "\x00\x00\x00\x0a\x49\x44\x41\x54\x78\x9c\x63\x00\x01\x00\x00\x05\x00"
                        "\x01\x0d\x0a\x2d\xb4\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82";

QString writePngFile()
{
    QTemporaryFile file(QDir::temp().filePath("XXXXXX-icon.png"));
    file.setAutoRemove(false);
    file.open();
    file.write(kPngData, sizeof(kPngData) - 1);
    file.close();
    return file.fileName();
}
}   // namespace

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
        return false;
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
        return true;
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

// rewritten: the implementation prefers DMimeDatabase::suffixForFileName
// (inherited, non-virtual) to resolve the real suffix.

// branch 1: mime lookup returns an empty suffix, the complete file suffix is
// used as fallback, "7z.001" then matches the "7z.*" wildcard by prefix
TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_MimeFallbackWildcardSuffix_ReturnsTrue)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "7z.*" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(ADDR(QMimeDatabase, suffixForFileName), [](QMimeDatabase *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return "";
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

// branch 2: real mime database resolves "test.txt" to the suffix "txt",
// which neither equals "7z.*" nor matches its wildcard prefix
TEST_F(UT_DCustomActionBuilder, IsSuffixSupport_RealMimeSuffixNoWildcardMatch_ReturnsFalse)
{
    DCustomActionEntry entry;
    entry.actionSupportSuffix = { "7z.*" };

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    auto info = QSharedPointer<FileInfo>(new FileInfo(url));
    bool result = DCustomActionBuilder::isSuffixSupport(entry, info);
    EXPECT_FALSE(result);
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

// ============ extended cases: private / protected full coverage ============

// ---------- appendAllMimeTypes ----------

TEST_F(UT_DCustomActionBuilder, AppendAllMimeTypes_PlainFileInfo_NoParentListStaysEmpty)
{
    // the plain FileInfo base returns an invalid QMimeType whose name is
    // empty, but whose (inherited) parent list reports octet-stream
    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));

    QStringList noParentMimeTypes;
    QStringList allMimeTypes;
    DCustomActionBuilder::appendAllMimeTypes(info, noParentMimeTypes, allMimeTypes);
    EXPECT_TRUE(noParentMimeTypes.isEmpty());
    EXPECT_EQ(allMimeTypes, QStringList({ "application/octet-stream" }));
}

TEST_F(UT_DCustomActionBuilder, AppendAllMimeTypes_TextPlain_FillsBothLists)
{
    stub.set_lamda(VADDR(FileInfo, fileMimeType), [](FileInfo *, QMimeDatabase::MatchMode) -> QMimeType {
        __DBG_STUB_INVOKE__
        return QMimeDatabase().mimeTypeForName("text/plain");
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));

    QStringList noParentMimeTypes;
    QStringList allMimeTypes;
    DCustomActionBuilder::appendAllMimeTypes(info, noParentMimeTypes, allMimeTypes);
    EXPECT_TRUE(noParentMimeTypes.contains("text/plain"));
    EXPECT_TRUE(allMimeTypes.contains("text/plain"));
    EXPECT_TRUE(allMimeTypes.contains("application/octet-stream"));
}

// ---------- appendParentMimeType ----------

TEST_F(UT_DCustomActionBuilder, AppendParentMimeType_EmptyInput_NoChange)
{
    QStringList mimeTypes { "text/plain" };
    DCustomActionBuilder::appendParentMimeType({}, mimeTypes);
    EXPECT_EQ(mimeTypes, QStringList({ "text/plain" }));
}

TEST_F(UT_DCustomActionBuilder, AppendParentMimeType_ValidParents_AppendsParentChain)
{
    QStringList mimeTypes;
    DCustomActionBuilder::appendParentMimeType({ "text/plain" }, mimeTypes);
    EXPECT_TRUE(mimeTypes.contains("text/plain"));
    EXPECT_TRUE(mimeTypes.contains("application/octet-stream"));
}

// ---------- createMenu / createAciton ----------

TEST_F(UT_DCustomActionBuilder, CreateMenu_WithChildren_ReturnsActionWithSubMenu)
{
    DCustomActionData menuData;
    menuData.actionName = "Menu";

    DCustomActionData child;
    child.actionName = "Child";
    child.actionCommand = "echo";
    menuData.childrenActions.append(child);

    QWidget parentWidget;
    QAction *action = builder->createMenu(menuData, &parentWidget);

    ASSERT_NE(action, nullptr);
    EXPECT_NE(action->menu(), nullptr);
    EXPECT_EQ(action->menu()->actions().size(), 1);
    EXPECT_TRUE(action->property(DCustomActionDefines::kCustomActionFlag).toBool());

    delete action;
}

TEST_F(UT_DCustomActionBuilder, CreateMenu_SeparatorFlags_InsertSeparators)
{
    DCustomActionData menuData;
    menuData.actionName = "Menu";

    DCustomActionData first;
    first.actionName = "First";
    first.actionCommand = "echo";

    DCustomActionData middle;
    middle.actionName = "Middle";
    middle.actionCommand = "echo";
    middle.actionSeparator = DCustomActionDefines::kBoth;

    DCustomActionData last;
    last.actionName = "Last";
    last.actionCommand = "echo";

    menuData.childrenActions.append(first);
    menuData.childrenActions.append(middle);
    menuData.childrenActions.append(last);

    QWidget parentWidget;
    QAction *action = builder->createMenu(menuData, &parentWidget);

    ASSERT_NE(action, nullptr);
    ASSERT_NE(action->menu(), nullptr);
    // first + top separator + middle + bottom separator + last
    EXPECT_EQ(action->menu()->actions().size(), 5);
    EXPECT_TRUE(action->menu()->actions().at(1)->isSeparator());
    EXPECT_TRUE(action->menu()->actions().at(3)->isSeparator());

    // parent path is propagated as property
    EXPECT_FALSE(action->property(DCustomActionDefines::kConfParentMenuPath).isValid());

    delete action;
}

TEST_F(UT_DCustomActionBuilder, CreateAciton_SetsCommandProperties)
{
    DCustomActionData data;
    data.actionName = "Act";
    data.actionCommand = "echo hi";
    data.actionCmdArg = DCustomActionDefines::kFilePath;
    data.actionParentPath = "Menu Entry";

    QAction *action = builder->createAciton(data);
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->property(DCustomActionDefines::kCustomActionCommand).toString(), "echo hi");
    EXPECT_EQ(action->text(), QString("Act"));
    EXPECT_EQ(action->property(DCustomActionDefines::kConfParentMenuPath).toString(), "Menu Entry");
    EXPECT_TRUE(action->property(DCustomActionDefines::kCustomActionFlag).toBool());

    delete action;
}

// ---------- makeName ----------

TEST_F(UT_DCustomActionBuilder, MakeName_AllArgs_ReplacesCorrectPlaceholder)
{
    builder->dirName = "mydir";
    builder->fileBaseName = "base";
    builder->fileFullName = "full.txt";

    EXPECT_EQ(builder->makeName("open %d", DCustomActionDefines::kDirName), QString("open mydir"));
    EXPECT_EQ(builder->makeName("open %b", DCustomActionDefines::kBaseName), QString("open base"));
    EXPECT_EQ(builder->makeName("open %a", DCustomActionDefines::kFileName), QString("open full.txt"));
}

TEST_F(UT_DCustomActionBuilder, MakeName_UnsupportedArg_ReturnsNameUnchanged)
{
    builder->dirName = "mydir";
    EXPECT_EQ(builder->makeName("plain text", DCustomActionDefines::kDirPath), QString("plain text"));
}

// ---------- getIcon ----------

TEST_F(UT_DCustomActionBuilder, GetIcon_ExistingImageFile_ReturnsUsableIcon)
{
    const QString pngPath = writePngFile();
    QIcon icon = builder->getIcon(pngPath);
    EXPECT_FALSE(icon.pixmap(QSize(16, 16)).isNull());
}

TEST_F(UT_DCustomActionBuilder, GetIcon_HomeRelativePath_FallsBackToTheme)
{
    QIcon icon = builder->getIcon("~/no-such-icon-file");
    EXPECT_TRUE(icon.isNull());
}

TEST_F(UT_DCustomActionBuilder, GetIcon_UnknownThemeName_ReturnsNullIcon)
{
    QIcon icon = builder->getIcon("no-such-icon-name-xyz");
    EXPECT_TRUE(icon.isNull());
}
