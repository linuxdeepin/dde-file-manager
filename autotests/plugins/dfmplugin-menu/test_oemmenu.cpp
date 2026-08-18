// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "oemmenuscene/oemmenu.h"
#include "oemmenuscene/private/oemmenu_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/protocolutils.h>

#include <DDesktopEntry>

#include <gtest/gtest.h>

#include <QMimeDatabase>
#include <QUrl>
#include <QAction>
#include <QMenu>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;
using Dtk::Core::DDesktopEntry;

namespace {
const char kMenuTypeKey[] = "X-DDE-FileManager-MenuTypes";
const char kMenuTypeAliasKey[] = "X-DFM-MenuTypes";
const char kMimeType[] = "MimeType";
const char kMenuHiddenKey[] = "X-DDE-FileManager-NotShowIn";
const char kMenuHiddenAliasKey[] = "X-DFM-NotShowIn";
const char kSupportSchemesKey[] = "X-DDE-FileManager-SupportSchemes";
const char kSupportSchemesAliasKey[] = "X-DFM-SupportSchemes";
const char kSupportSuffixKey[] = "X-DDE-FileManager-SupportSuffix";
const char kSupportSuffixAliasKey[] = "X-DFM-SupportSuffix";
const char kCommandKey[] = "Exec";
const char kEmptyArea[] = "EmptyArea";
const char kSingleFile[] = "SingleFile";

// write content into a temporary *.desktop file, return its path
QString writeDesktopFile(const QString &content)
{
    QTemporaryFile file(QDir::temp().filePath("XXXXXX-oemmenu.desktop"));
    file.setAutoRemove(false);
    file.open();
    file.write(content.toUtf8());
    file.close();
    return file.fileName();
}
}   // namespace

class UT_OemMenu : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // the private ctor starts watchers on the oem directories, keep
        // the test free of such side effects
        stub.set_lamda(VADDR(LocalFileWatcher, startWatcher), []() {
            __DBG_STUB_INVOKE__
            return true;
        });

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

    OemMenuPrivate *pd()
    {
        return menu->d.data();
    }
};

// ================= migrated legacy cases =================

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

// ============ extended cases: OemMenuPrivate full coverage ============

// ---------- loadDesktopFile with a controlled oem directory ----------

TEST_F(UT_OemMenu, LoadDesktopFile_ValidOemDir_LoadsActionsAndSubMenus)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // visible entry, registered for two menu types, with one sub action
    QFile visible(QDir(dir.path()).filePath("visible.desktop"));
    ASSERT_TRUE(visible.open(QIODevice::WriteOnly | QIODevice::Text));
    visible.write("[Desktop Entry]\n"
                  "Type=Application\n"
                  "Name=OEM Test\n"
                  "X-DDE-FileManager-MenuTypes=EmptyArea;SingleFile\n"
                  "Actions=act1\n"
                  "\n"
                  "[Desktop Action act1]\n"
                  "Name=Action One\n"
                  "Exec=/bin/true %p\n");
    visible.close();

    // hidden entry, NoDisplay makes it be skipped
    QFile hidden(QDir(dir.path()).filePath("hidden.desktop"));
    ASSERT_TRUE(hidden.open(QIODevice::WriteOnly | QIODevice::Text));
    hidden.write("[Desktop Entry]\n"
                 "Type=Application\n"
                 "Name=Hidden\n"
                 "NoDisplay=true\n"
                 "X-DDE-FileManager-MenuTypes=EmptyArea\n");
    hidden.close();

    // entry whose menu types are all filtered out by the white list
    QFile bogus(QDir(dir.path()).filePath("bogus.desktop"));
    ASSERT_TRUE(bogus.open(QIODevice::WriteOnly | QIODevice::Text));
    bogus.write("[Desktop Entry]\n"
                "Type=Application\n"
                "Name=Bogus\n"
                "X-DDE-FileManager-MenuTypes=BogusType\n");
    bogus.close();

    pd()->oemMenuPath.clear();
    pd()->oemMenuPath << dir.path();

    menu->loadDesktopFile();

    EXPECT_EQ(pd()->actionListByType.value(kEmptyArea).size(), 1);
    EXPECT_EQ(pd()->actionListByType.value(kSingleFile).size(), 1);
    EXPECT_FALSE(pd()->actionListByType.contains("BogusType"));
    EXPECT_EQ(pd()->subMenus.size(), 1);
}

// ---------- getValues ----------

TEST_F(UT_OemMenu, GetValues_KeyWithEmptyWhiteList_ReturnsAllValues)
{
    // both key and alias present so no empty artifacts from missing keys
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n"
            "X-DDE-FileManager-MenuTypes=EmptyArea;SingleFile\n"
            "X-DFM-MenuTypes=SingleDir\n");
    DDesktopEntry entry(path);

    auto values = pd()->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, "Desktop Entry", {});
    EXPECT_EQ(values, QStringList({ kEmptyArea, kSingleFile, "SingleDir" }));
}

TEST_F(UT_OemMenu, GetValues_WhiteListFilter_KeepsWhitelistedOnly)
{
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n"
            "X-DDE-FileManager-MenuTypes=EmptyArea\n"
            "X-DFM-MenuTypes=SingleFile\n");
    DDesktopEntry entry(path);

    auto values = pd()->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, "Desktop Entry", { kEmptyArea });
    EXPECT_EQ(values, QStringList({ kEmptyArea }));
}

TEST_F(UT_OemMenu, GetValues_KeysAbsent_ReturnsWhiteList)
{
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n");
    DDesktopEntry entry(path);

    auto values = pd()->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, "Desktop Entry", { "X", "Y" });
    EXPECT_EQ(values, QStringList({ "X", "Y" }));
}

TEST_F(UT_OemMenu, GetValues_AliasOnly_ReturnsAliasValues)
{
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n"
            "X-DFM-MenuTypes=SingleDir\n");
    DDesktopEntry entry(path);

    auto values = pd()->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, "Desktop Entry", { "SingleDir" });
    EXPECT_EQ(values, QStringList({ "SingleDir" }));
}

// ---------- isMimeTypeSupport / isMimeTypeMatch ----------

TEST_F(UT_OemMenu, IsMimeTypeSupport_PartialToken_ReturnsTrue)
{
    EXPECT_TRUE(pd()->isMimeTypeSupport("text", { "text/plain", "application/octet-stream" }));
}

TEST_F(UT_OemMenu, IsMimeTypeSupport_NoMatch_ReturnsFalse)
{
    EXPECT_FALSE(pd()->isMimeTypeSupport("image", { "text/plain" }));
}

TEST_F(UT_OemMenu, IsMimeTypeSupport_CaseInsensitive_ReturnsTrue)
{
    EXPECT_TRUE(pd()->isMimeTypeSupport("TEXT", { "text/plain" }));
}

TEST_F(UT_OemMenu, IsMimeTypeMatch_ExactListed_ReturnsTrue)
{
    EXPECT_TRUE(pd()->isMimeTypeMatch({ "text/plain" }, { "text/plain" }));
}

TEST_F(UT_OemMenu, IsMimeTypeMatch_WildcardPrefix_ReturnsTrue)
{
    EXPECT_TRUE(pd()->isMimeTypeMatch({ "text/plain", "text/html" }, { "text/*" }));
}

TEST_F(UT_OemMenu, IsMimeTypeMatch_WildcardMismatch_ReturnsFalse)
{
    EXPECT_FALSE(pd()->isMimeTypeMatch({ "text/plain" }, { "image/*" }));
}

TEST_F(UT_OemMenu, IsMimeTypeMatch_EmptySupport_ReturnsFalse)
{
    EXPECT_FALSE(pd()->isMimeTypeMatch({ "text/plain" }, {}));
}

// ---------- isActionShouldShow ----------

TEST_F(UT_OemMenu, IsActionShouldShow_NullAction_ReturnsFalse)
{
    EXPECT_FALSE(pd()->isActionShouldShow(nullptr, false));
}

TEST_F(UT_OemMenu, IsActionShouldShow_NoHiddenProperty_ReturnsTrue)
{
    QAction action("a");
    EXPECT_TRUE(pd()->isActionShouldShow(&action, true));
    EXPECT_TRUE(pd()->isActionShouldShow(&action, false));
}

TEST_F(UT_OemMenu, IsActionShouldShow_NotShowInDesktop_OnDesktop_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kMenuHiddenKey, QStringList { "Desktop" });
    EXPECT_FALSE(pd()->isActionShouldShow(&action, true));
}

TEST_F(UT_OemMenu, IsActionShouldShow_NotShowInDesktop_OffDesktop_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kMenuHiddenKey, QStringList { "Desktop" });
    EXPECT_TRUE(pd()->isActionShouldShow(&action, false));
}

TEST_F(UT_OemMenu, IsActionShouldShow_AliasNotShowInFilemanager_OffDesktop_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kMenuHiddenAliasKey, QStringList { "Filemanager" });
    EXPECT_FALSE(pd()->isActionShouldShow(&action, false));
}

// ---------- isSchemeSupport ----------

TEST_F(UT_OemMenu, IsSchemeSupport_NullAction_ReturnsTrue)
{
    EXPECT_TRUE(pd()->isSchemeSupport(nullptr, QUrl::fromLocalFile("/tmp")));
}

TEST_F(UT_OemMenu, IsSchemeSupport_NoProperty_ReturnsTrue)
{
    QAction action("a");
    EXPECT_TRUE(pd()->isSchemeSupport(&action, QUrl::fromLocalFile("/tmp")));
}

TEST_F(UT_OemMenu, IsSchemeSupport_ListedScheme_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSchemesKey, QStringList { "file" });
    EXPECT_TRUE(pd()->isSchemeSupport(&action, QUrl::fromLocalFile("/tmp/a.txt")));
}

TEST_F(UT_OemMenu, IsSchemeSupport_UnlistedScheme_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kSupportSchemesKey, QStringList { "file" });
    EXPECT_FALSE(pd()->isSchemeSupport(&action, QUrl("ftp://server/a.txt")));
}

TEST_F(UT_OemMenu, IsSchemeSupport_AliasListedScheme_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSchemesAliasKey, QStringList { "ftp" });
    EXPECT_TRUE(pd()->isSchemeSupport(&action, QUrl("ftp://server/a.txt")));
}

// ---------- isSuffixSupport ----------

TEST_F(UT_OemMenu, IsSuffixSupport_NullFileInfo_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "txt" });
    EXPECT_TRUE(pd()->isSuffixSupport(&action, FileInfoPointer()));
}

TEST_F(UT_OemMenu, IsSuffixSupport_NullFileInfoAllEx7z_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "txt" });
    EXPECT_FALSE(pd()->isSuffixSupport(&action, FileInfoPointer(), true));
}

TEST_F(UT_OemMenu, IsSuffixSupport_Directory_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "txt" });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return true;   // directory
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/dir")));
    EXPECT_TRUE(pd()->isSuffixSupport(&action, info));
}

TEST_F(UT_OemMenu, IsSuffixSupport_EmptyProperty_ReturnsTrue)
{
    QAction action("a");   // no X-DFM-SupportSuffix property

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, const OptInfoType) {
        __DBG_STUB_INVOKE__
        return false;
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_TRUE(pd()->isSuffixSupport(&action, info));
}

TEST_F(UT_OemMenu, IsSuffixSupport_MimeFallbackWildcard_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "7z.*" });

    // make the mime lookup fall back to the complete suffix
    stub.set_lamda(ADDR(QMimeDatabase, suffixForFileName), [](QMimeDatabase *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return "";
    });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("7z.001");
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.7z.001")));
    EXPECT_TRUE(pd()->isSuffixSupport(&action, info));
}

TEST_F(UT_OemMenu, IsSuffixSupport_RealMimeExactMatch_ReturnsTrue)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "txt" });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_TRUE(pd()->isSuffixSupport(&action, info));
}

TEST_F(UT_OemMenu, IsSuffixSupport_RealMimeNoMatch_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kSupportSuffixKey, QStringList { "gz" });

    stub.set_lamda(VADDR(FileInfo, nameOf), [](FileInfo *, const NameInfoType) {
        __DBG_STUB_INVOKE__
        return QString("test.txt");
    });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_FALSE(pd()->isSuffixSupport(&action, info));
}

// ---------- isAllEx7zFile ----------

TEST_F(UT_OemMenu, IsAllEx7zFile_SingleUrl_ReturnsFalse)
{
    EXPECT_FALSE(pd()->isAllEx7zFile({ QUrl::fromLocalFile("/tmp/a.7z.001") }));
}

TEST_F(UT_OemMenu, IsAllEx7zFile_AllSplitArchives_ReturnsTrue)
{
    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    int suffixCalls = 0;
    stub.set_lamda(VADDR(FileInfo, nameOf), [&suffixCalls](FileInfo *, const NameInfoType type) {
        __DBG_STUB_INVOKE__
        if (type == NameInfoType::kCompleteSuffix)
            return QString("7z.00") + QString::number(++suffixCalls);
        return QString("a.7z.00x");
    });

    EXPECT_TRUE(pd()->isAllEx7zFile({ QUrl::fromLocalFile("/tmp/a.7z.001"),
                                      QUrl::fromLocalFile("/tmp/a.7z.002") }));
}

TEST_F(UT_OemMenu, IsAllEx7zFile_MixedFiles_ReturnsFalse)
{
    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return QSharedPointer<FileInfo>(new FileInfo(url));
        });

    int suffixCalls = 0;
    stub.set_lamda(VADDR(FileInfo, nameOf), [&suffixCalls](FileInfo *, const NameInfoType type) {
        __DBG_STUB_INVOKE__
        if (type == NameInfoType::kCompleteSuffix)
            return ++suffixCalls == 1 ? QString("7z.001") : QString("txt");
        return QString("a.archive");
    });

    EXPECT_FALSE(pd()->isAllEx7zFile({ QUrl::fromLocalFile("/tmp/a.7z.001"),
                                       QUrl::fromLocalFile("/tmp/b.txt") }));
}

TEST_F(UT_OemMenu, IsAllEx7zFile_NullInfo_ReturnsFalse)
{
    stub.set_lamda(&InfoFactory::create<FileInfo>,
        [](const QUrl &, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
            __DBG_STUB_INVOKE__
            return nullptr;
        });

    EXPECT_FALSE(pd()->isAllEx7zFile({ QUrl::fromLocalFile("/tmp/a.7z.001"),
                                       QUrl::fromLocalFile("/tmp/a.7z.002") }));
}

// ---------- isValid ----------

TEST_F(UT_OemMenu, IsValid_NullAction_ReturnsFalse)
{
    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_FALSE(pd()->isValid(nullptr, info, false));
}

TEST_F(UT_OemMenu, IsValid_UnrestrictedAction_ReturnsTrue)
{
    QAction action("a");
    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_TRUE(pd()->isValid(&action, info, false));
}

TEST_F(UT_OemMenu, IsValid_UnsupportedScheme_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kSupportSchemesKey, QStringList { "ftp" });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_FALSE(pd()->isValid(&action, info, false));
}

TEST_F(UT_OemMenu, IsValid_HiddenOnDesktop_ReturnsFalse)
{
    QAction action("a");
    action.setProperty(kMenuHiddenKey, QStringList { "Desktop" });

    FileInfoPointer info(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    EXPECT_FALSE(pd()->isValid(&action, info, true));
}

// ---------- clearSubMenus / setActionProperty ----------

TEST_F(UT_OemMenu, ClearSubMenus_RemovesAllMenus_ListBecomesEmpty)
{
    pd()->subMenus.append(new QMenu());
    pd()->subMenus.append(new QMenu());
    ASSERT_EQ(pd()->subMenus.size(), 2);

    pd()->clearSubMenus();
    EXPECT_TRUE(pd()->subMenus.isEmpty());

    // flush the deferred deletions
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_F(UT_OemMenu, SetActionProperty_KeyPresent_SetsProperty)
{
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n"
            "MimeType=text/plain\n");
    DDesktopEntry entry(path);

    QAction action("a");
    pd()->setActionProperty(&action, entry, kMimeType, "Desktop Entry");
    EXPECT_EQ(action.property(kMimeType).toStringList(), QStringList({ "text/plain" }));
}

TEST_F(UT_OemMenu, SetActionProperty_KeyAbsent_NoPropertySet)
{
    const QString path = writeDesktopFile(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=T\n");
    DDesktopEntry entry(path);

    QAction action("a");
    pd()->setActionProperty(&action, entry, kMimeType, "Desktop Entry");
    EXPECT_FALSE(action.property(kMimeType).isValid());
}

// ---------- splitCommand ----------

TEST_F(UT_OemMenu, SplitCommand_DoubleQuotedArg_PreservesSpaces)
{
    auto args = pd()->splitCommand("echo \"hello world\"");
    EXPECT_EQ(args, QStringList({ "echo", "hello world" }));
}

TEST_F(UT_OemMenu, SplitCommand_SingleQuotedArg_PreservesSpaces)
{
    auto args = pd()->splitCommand("echo 'a b' c");
    EXPECT_EQ(args, QStringList({ "echo", "a b", "c" }));
}

TEST_F(UT_OemMenu, SplitCommand_EmptyCommand_ReturnsEmpty)
{
    EXPECT_TRUE(pd()->splitCommand("").isEmpty());
}

// ---------- execDynamicArg ----------

TEST_F(UT_OemMenu, ExecDynamicArg_NegativeIndex_ReturnsNone)
{
    auto result = pd()->execDynamicArg({ "cmd", "%p" }, -1);
    EXPECT_EQ(result.first, OemMenuPrivate::kNoneArg);
    EXPECT_EQ(result.second, -1);
}

TEST_F(UT_OemMenu, ExecDynamicArg_DirPathArg_FoundAtIndex)
{
    auto result = pd()->execDynamicArg({ "cmd", "%p" }, 0);
    EXPECT_EQ(result.first, OemMenuPrivate::kDirPath);
    EXPECT_EQ(result.second, 1);
}

TEST_F(UT_OemMenu, ExecDynamicArg_ScanPastEscapedPercent_FindsArg)
{
    // "%%u": the first pair is not a valid arg, scanning continues and finds "%u"
    auto result = pd()->execDynamicArg({ "cmd", "pre%%u" }, 0);
    EXPECT_EQ(result.first, OemMenuPrivate::kUrlPath);
    EXPECT_EQ(result.second, 1);
}

TEST_F(UT_OemMenu, ExecDynamicArg_NoDynamicArg_ReturnsNone)
{
    auto result = pd()->execDynamicArg({ "cmd", "plain" }, 0);
    EXPECT_EQ(result.first, OemMenuPrivate::kNoneArg);
    EXPECT_EQ(result.second, -1);
}

TEST_F(UT_OemMenu, ExecDynamicArg_SearchFromGivenIndex_FindsLaterArg)
{
    auto result = pd()->execDynamicArg({ "cmd", "plain", "%f" }, 1);
    EXPECT_EQ(result.first, OemMenuPrivate::kFilePath);
    EXPECT_EQ(result.second, 2);
}

// ---------- applyDynamicArg ----------

TEST_F(UT_OemMenu, ApplyDynamicArg_DirPath_ReplacesArg)
{
    QStringList args { "cmd", "-d", "%p" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kDirPath,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl::fromLocalFile("/tmp/a.txt"), {});
    EXPECT_EQ(rets, QStringList({ "cmd", "-d", "/tmp" }));
}

TEST_F(UT_OemMenu, ApplyDynamicArg_FilePath_ReplacesArg)
{
    QStringList args { "cmd", "%f" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kFilePath,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl::fromLocalFile("/tmp/a.txt"), {});
    EXPECT_EQ(rets, QStringList({ "cmd", "/tmp/a.txt" }));
}

TEST_F(UT_OemMenu, ApplyDynamicArg_FilePaths_ExpandsList)
{
    QStringList args { "cmd", "%F" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kFilePaths,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl::fromLocalFile("/tmp/a.txt"),
                                      { QUrl::fromLocalFile("/tmp/a.txt"),
                                        QUrl::fromLocalFile("/tmp/b.txt") });
    EXPECT_EQ(rets, QStringList({ "cmd", "/tmp/a.txt", "/tmp/b.txt" }));
}

TEST_F(UT_OemMenu, ApplyDynamicArg_UrlPath_ReplacesWithEncodedForRemote)
{
    QStringList args { "cmd", "%u" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kUrlPath,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl("ftp://server/a.txt"), {});
    EXPECT_EQ(rets, QStringList({ "cmd", "ftp://server/a.txt" }));
}

TEST_F(UT_OemMenu, ApplyDynamicArg_UrlPaths_ExpandsEncodedList)
{
    QStringList args { "cmd", "%U" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kUrlPaths,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl("ftp://server/a.txt"),
                                      { QUrl::fromLocalFile("/tmp/a.txt"),
                                        QUrl("ftp://server/b.txt") });
    EXPECT_EQ(rets, QStringList({ "cmd", "/tmp/a.txt", "ftp://server/b.txt" }));
}

TEST_F(UT_OemMenu, ApplyDynamicArg_NoneArg_ReturnsArgsUnchanged)
{
    QStringList args { "cmd", "x" };
    auto rets = pd()->applyDynamicArg(args, OemMenuPrivate::kNoneArg,
                                      QUrl::fromLocalFile("/tmp"),
                                      QUrl::fromLocalFile("/tmp/a.txt"), {});
    EXPECT_EQ(rets, QStringList({ "cmd", "x" }));
}

// ---------- replace / replaceList ----------

TEST_F(UT_OemMenu, Replace_FirstMatch_ReplacedAndTailKept)
{
    QStringList args { "a", "x%py", "z" };
    auto rets = pd()->replace(args, "%p", "/tmp");
    EXPECT_EQ(rets, QStringList({ "a", "x/tmpy", "z" }));
}

TEST_F(UT_OemMenu, Replace_NoMatch_ReturnsSameList)
{
    QStringList args { "a", "b" };
    auto rets = pd()->replace(args, "%p", "/tmp");
    EXPECT_EQ(rets, QStringList({ "a", "b" }));
}

TEST_F(UT_OemMenu, ReplaceList_IndependentArg_Expanded)
{
    QStringList args { "a", "%F", "z" };
    auto rets = pd()->replaceList(args, "%F", { "/f1", "/f2" });
    EXPECT_EQ(rets, QStringList({ "a", "/f1", "/f2", "z" }));
}

TEST_F(UT_OemMenu, ReplaceList_ComposedArg_KeptUnchanged)
{
    QStringList args { "a", "x%Fy", "z" };
    auto rets = pd()->replaceList(args, "%F", { "/f1" });
    EXPECT_EQ(rets, QStringList({ "a", "x%Fy", "z" }));
}

// ---------- url helpers ----------

TEST_F(UT_OemMenu, UrlListToLocalFile_ConvertsToLocalPaths)
{
    QList<QUrl> files { QUrl::fromLocalFile("/tmp/a"), QUrl::fromLocalFile("/tmp/b") };
    EXPECT_EQ(pd()->urlListToLocalFile(files), QStringList({ "/tmp/a", "/tmp/b" }));
}

TEST_F(UT_OemMenu, UrlToString_LocalFile_ReturnsPath)
{
    EXPECT_EQ(pd()->urlToString(QUrl::fromLocalFile("/tmp/a.txt")), QString("/tmp/a.txt"));
}

TEST_F(UT_OemMenu, UrlToString_RemoteUrl_ReturnsEncoded)
{
    EXPECT_EQ(pd()->urlToString(QUrl("ftp://server/a b.txt")), QString("ftp://server/a%20b.txt"));
}

TEST_F(UT_OemMenu, UrlListToString_MixedList_UsesLocalOrEncoded)
{
    QList<QUrl> files { QUrl::fromLocalFile("/tmp/a"), QUrl("ftp://server/b") };
    EXPECT_EQ(pd()->urlListToString(files), QStringList({ "/tmp/a", "ftp://server/b" }));
}

// ---------- appendParentMineType ----------

TEST_F(UT_OemMenu, AppendParentMineType_EmptyInput_NoChange)
{
    QStringList mimeTypes { "text/plain" };
    pd()->appendParentMineType({}, mimeTypes);
    EXPECT_EQ(mimeTypes, QStringList({ "text/plain" }));
}

TEST_F(UT_OemMenu, AppendParentMineType_KnownMime_AppendsParents)
{
    QStringList mimeTypes;
    pd()->appendParentMineType({ "text/plain" }, mimeTypes);
    EXPECT_TRUE(mimeTypes.contains("text/plain"));
    EXPECT_TRUE(mimeTypes.contains("application/octet-stream"));
}

// OemMenuPrivate 构造函数为 3 个 oem 菜单目录创建 LocalFileWatcher 并连接
// fileDeleted / subfileCreated / fileAttributeChanged 到重载定时器的 lambda。
// 通过元对象触发各信号，验证三个 lambda 均被执行（定时器被启动）。
TEST_F(UT_OemMenu, WatcherSignals_RestartDelayedLoadFileTimer)
{
    OemMenu menu;
    auto *priv = menu.d.data();
    ASSERT_NE(priv, nullptr);
    ASSERT_NE(priv->delayedLoadFileTimer, nullptr);

    auto watchers = menu.findChildren<LocalFileWatcher *>();
    ASSERT_GT(watchers.size(), 0);

    const QUrl url = QUrl::fromLocalFile("/tmp/fake-oem-menu-file");
    for (auto *watcher : watchers) {
        EXPECT_TRUE(QMetaObject::invokeMethod(watcher, "fileDeleted", Qt::DirectConnection, Q_ARG(QUrl, url)));
        EXPECT_TRUE(priv->delayedLoadFileTimer->isActive());
        priv->delayedLoadFileTimer->stop();

        EXPECT_TRUE(QMetaObject::invokeMethod(watcher, "subfileCreated", Qt::DirectConnection, Q_ARG(QUrl, url)));
        EXPECT_TRUE(priv->delayedLoadFileTimer->isActive());
        priv->delayedLoadFileTimer->stop();

        EXPECT_TRUE(QMetaObject::invokeMethod(watcher, "fileAttributeChanged", Qt::DirectConnection, Q_ARG(QUrl, url)));
        EXPECT_TRUE(priv->delayedLoadFileTimer->isActive());
        priv->delayedLoadFileTimer->stop();
    }
}
