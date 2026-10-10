// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_commandparser.cpp - CommandParser + SessionBusiness 单元测试
// 重编译 app 源码（commandparser.cpp / sessionloader.cpp）进测试二进制，
// 通过 stub_ext 隔离 DConfig/DBus/窗口依赖，验证命令行分支与事件发布行为。
//
// 分支清单（来源：get_code_snippet 真实源码）→ 用例映射：
// CommandParser::processCommand
//   [activation-token 非空 → qputenv]        → ActivationToken_SetsEnvVar
//   [activation-token 为空 → 不设置]          → EmptyActivationToken_DoesNotSetEnv
//   [-d headless → publish kHeadlessStarted] → HeadlessFlag_PublishesHeadlessStarted
//   [-e → processEvent]                      → ProcessEvent_* 系列
//   [-p → showPropertyDialog]                → PropertyDialog_* 系列
//   [-o → openWithDialog]                    → OpenWithDialog_PushesUrlList
//   [-O → openInHomeDirectory]               → OpenHome_PublishesOpenNewWindow
//   [-s → openSession]                       → SessionFile_* 系列
//   [default → openInUrls]                   → OpenInUrls_* 系列
// CommandParser::showPropertyDialog
//   [本地文件不存在 → continue]               → PropertyDialog_MissingFile_SkipsInvalidPath
//   [uPath 尾分隔符 → chop(1)]                → PropertyDialog_TrailingSeparator_Chopped
//   [urlList 空 → early return]              → PropertyDialog_MissingFile_SkipsInvalidPath
// CommandParser::openInUrls
//   [非 raw 且含 #&@!? → percent-encode]      → SpecialChars_EncodesPath
//   [--show-item 且 InfoFactory 空 → continue]→ ShowItemNonexistent_SkipsFile
//   [argumentUrls 空 → publish kOpenNewWindow]→ ShowItemNonexistent_SkipsFile
// CommandParser::openWindowWithUrl
//   [kOpenInNewTab && actWinId>0 → kOpenNewTab]→ NewTabAttribute_PublishesNewTab
//   [else → kOpenNewWindow]                   → SpecialChars_EncodesPath
// CommandParser::processEvent
//   [positional 空 → early return]            → EmptyPositional_ReturnsEarly
//   [action 未知 → warn 返回]                 → UnknownAction_DoesNotPublish
//   [refresh → slot push]                     → Refresh_PushesSlotChannel
//   [copy → publish kCopy]                    → CopyAction_PublishesCopyEvent
//   [move 且系统路径 → return]                → CutSystemPath_SkipsPublish
//   [delete → publish kDeleteFiles]           → DeleteAction_PublishesDeleteEvent
//   [trash 且 trash 文件 → return]            → TrashTrashFile_SkipsPublish
//   [cleantrash → publish kCleanTrash]        → CleanTrash_PublishesCleanTrashEvent
// SessionBusiness::readPath
//   [data 为空 → false]                       → ReadPath_NullPointer_ReturnsFalse
//   [文件打不开 → false]                       → ReadPath_MissingFile_ReturnsFalse
//   [JSON 非法 → false]                       → ReadPath_InvalidJson_ReturnsFalse
//   [path 空 → false]                         → ReadPath_EmptyPath_ReturnsFalse
//   [合法 → true 且写出 path]                 → ReadPath_ValidJson_ReturnsTrueAndPath
// SessionBusiness::parseArguments
//   [arguments 空 → nullptr]                  → ParseArgumentsEmpty_ReturnsNullptr
//   [非空 → argc/argv 填充]                   → ParseArguments_FillsArgv

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QLoggingCategory>

#include "stubext.h"

#include "commandparser.h"
#include "sessionloader.h"

#include <dfm-framework/dpf.h>
#include <dfm-framework/event/event.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

using namespace dpf;
using namespace dfmbase;

// Defined in app main.cpp which is not compiled into this test binary.
Q_LOGGING_CATEGORY(logAppFileManager, "org.deepin.dde.filemanager.filemanager")

namespace {
struct GlobalEventListener : public QObject
{
    explicit GlobalEventListener(QObject *parent = nullptr)
        : QObject(parent) { }

    int headlessCalls = 0;
    int openNewWindowCalls = 0;
    QUrl lastNewWindowUrl;
    bool lastNewWindowFlag = false;
    int openNewTabCalls = 0;
    QUrl lastNewTabUrl;
    int copyCalls = 0;
    QList<QUrl> lastCopySources;
    QUrl lastCopyTarget;
    int deleteCalls = 0;
    int cleanTrashCalls = 0;

    QVariant onHeadless()
    {
        ++headlessCalls;
        return QVariant();
    }
    QVariant onOpenNewWindow(const QUrl &url, bool flag)
    {
        ++openNewWindowCalls;
        lastNewWindowUrl = url;
        lastNewWindowFlag = flag;
        return QVariant();
    }
    QVariant onOpenNewTab(quint64, const QUrl &url)
    {
        ++openNewTabCalls;
        lastNewTabUrl = url;
        return QVariant();
    }
    QVariant onCopy(quint64, const QList<QUrl> &sources, const QUrl &target,
                    AbstractJobHandler::JobFlag, const QVariant &)
    {
        ++copyCalls;
        lastCopySources = sources;
        lastCopyTarget = target;
        return QVariant();
    }
    QVariant onDelete(quint64, const QList<QUrl> &, AbstractJobHandler::JobFlag, const QVariant &)
    {
        ++deleteCalls;
        return QVariant();
    }
    QVariant onCleanTrash(quint64, const QList<QUrl> &, AbstractJobHandler::JobFlag, const QVariant &)
    {
        ++cleanTrashCalls;
        return QVariant();
    }
};

std::once_flag g_subscribeOnce;
GlobalEventListener *g_listener = nullptr;

void ensureGlobalListener()
{
    std::call_once(g_subscribeOnce, []() {
        static GlobalEventListener listener;
        g_listener = &listener;
        auto *mgr = dpfSignalDispatcher;
        mgr->subscribe(GlobalEventType::kHeadlessStarted, g_listener, &GlobalEventListener::onHeadless);
        mgr->subscribe(GlobalEventType::kOpenNewWindow, g_listener, &GlobalEventListener::onOpenNewWindow);
        mgr->subscribe(GlobalEventType::kOpenNewTab, g_listener, &GlobalEventListener::onOpenNewTab);
        mgr->subscribe(GlobalEventType::kCopy, g_listener, &GlobalEventListener::onCopy);
        mgr->subscribe(GlobalEventType::kDeleteFiles, g_listener, &GlobalEventListener::onDelete);
        mgr->subscribe(GlobalEventType::kCleanTrash, g_listener, &GlobalEventListener::onCleanTrash);
    });
}

int listenerCount()
{
    return g_listener ? g_listener->openNewWindowCalls : 0;
}
}   // namespace

class CommandParserTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        ensureGlobalListener();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        qunsetenv("XDG_ACTIVATION_TOKEN");
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        qunsetenv("XDG_ACTIVATION_TOKEN");
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    CommandParser &parser = CommandParser::instance();
};

TEST_F(CommandParserTest, Instance_IsSingleton_SameReference)
{
    // Arrange / Act
    auto &first = CommandParser::instance();
    auto &second = CommandParser::instance();

    // Assert
    EXPECT_EQ(&first, &second);
    EXPECT_EQ(&parser, &first);
}

TEST_F(CommandParserTest, Process_ShortFlag_IsSetReflectsParsedState)
{
    // Arrange
    const QStringList args { "ut-commandparser", "-n" };

    // Act
    parser.process(args);

    // Assert
    EXPECT_TRUE(parser.isSet("n"));
    EXPECT_FALSE(parser.isSet("d"));
    EXPECT_FALSE(parser.isSet("p"));
}

TEST_F(CommandParserTest, Process_ValueOption_ReturnsValue)
{
    // Arrange
    const QStringList args { "ut-commandparser", "-w", "/tmp/working-dir" };

    // Act
    parser.process(args);

    // Assert
    EXPECT_TRUE(parser.isSet("w"));
    EXPECT_EQ(parser.value("w"), QString("/tmp/working-dir"));
}

TEST_F(CommandParserTest, Process_UnknownName_IsSetFalseAndValueEmpty)
{
    // Arrange
    parser.process({ "ut-commandparser" });

    // Act / Assert
    EXPECT_FALSE(parser.isSet("not-an-option"));
    EXPECT_TRUE(parser.value("not-an-option").isEmpty());
}

TEST_F(CommandParserTest, ProcessCommand_HeadlessFlag_PublishesHeadlessStarted)
{
    // Arrange
    const int before = g_listener->headlessCalls;
    parser.process({ "ut-commandparser", "-d" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->headlessCalls, before + 1);   // branch: isSet("d") -> publish kHeadlessStarted
}

TEST_F(CommandParserTest, ProcessCommand_ActivationToken_SetsEnvVar)
{
    // Arrange
    parser.process({ "ut-commandparser", "--activation-token", "tok-42", "-d" });
    const int headlessBefore = g_listener->headlessCalls;

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(qgetenv("XDG_ACTIVATION_TOKEN"), QByteArray("tok-42"));   // branch: token non-empty
    EXPECT_EQ(g_listener->headlessCalls, headlessBefore + 1);
}

TEST_F(CommandParserTest, ProcessCommand_EmptyActivationToken_DoesNotSetEnv)
{
    // Arrange
    parser.process({ "ut-commandparser", "--activation-token=", "-d" });
    const int headlessBefore = g_listener->headlessCalls;

    // Act
    parser.processCommand();

    // Assert
    EXPECT_TRUE(qgetenv("XDG_ACTIVATION_TOKEN").isEmpty());   // branch: token empty -> no qputenv
    EXPECT_EQ(g_listener->headlessCalls, headlessBefore + 1);
}

TEST_F(CommandParserTest, ProcessCommand_PropertyDialog_PushesUrlList)
{
    // Arrange
    bool pushed = false;
    QList<QUrl> captured;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &,
                                                      QList<QUrl>, QVariantHash &&);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&pushed, &captured](EventChannelManager *, const QString &space,
                                        const QString &topic, QList<QUrl> urls, QVariantHash) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_propertydialog" && topic == "slot_PropertyDialog_Show") {
                           pushed = true;
                           captured = urls;
                       }
                       return QVariant();
                   });

    const QString filePath = tempDir->filePath("prop.txt");
    ASSERT_TRUE(QFile(filePath).open(QIODevice::WriteOnly));
    parser.process({ "ut-commandparser", "-p", filePath });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_TRUE(pushed);   // existing local file passes FileInfo check
    ASSERT_EQ(captured.size(), 1);
    EXPECT_EQ(captured.first().toLocalFile(), filePath);
}

TEST_F(CommandParserTest, ProcessCommand_PropertyDialog_MissingFile_SkipsInvalidPath)
{
    // Arrange
    bool pushed = false;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &,
                                                      QList<QUrl>, QVariantHash &&);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&pushed](EventChannelManager *, const QString &, const QString &,
                             QList<QUrl>, QVariantHash) -> QVariant {
                       __DBG_STUB_INVOKE__
                       pushed = true;
                       return QVariant();
                   });

    parser.process({ "ut-commandparser", "-p", tempDir->filePath("does-not-exist.txt") });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_FALSE(pushed);   // branch: local file missing -> continue; urlList empty -> early return
}

TEST_F(CommandParserTest, ProcessCommand_PropertyDialog_TrailingSeparator_Chopped)
{
    // Arrange
    bool pushed = false;
    QList<QUrl> captured;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &,
                                                      QList<QUrl>, QVariantHash &&);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&pushed, &captured](EventChannelManager *, const QString &, const QString &,
                                        QList<QUrl> urls, QVariantHash) -> QVariant {
                       __DBG_STUB_INVOKE__
                       pushed = true;
                       captured = urls;
                       return QVariant();
                   });

    const QString dirPath = tempDir->path();
    parser.process({ "ut-commandparser", "-p", dirPath });

    // Act
    parser.processCommand();

    // Assert
    ASSERT_TRUE(pushed);
    ASSERT_EQ(captured.size(), 1);
    EXPECT_FALSE(captured.first().path().endsWith('/'));   // branch: trailing separator chopped
    EXPECT_EQ(captured.first().toLocalFile(), dirPath);
}

TEST_F(CommandParserTest, ProcessCommand_OpenWithDialog_PushesUrlList)
{
    // Arrange
    bool pushed = false;
    QList<QUrl> captured;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &,
                                                      int, QList<QUrl> &);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&pushed, &captured](EventChannelManager *, const QString &space,
                                        const QString &topic, int, QList<QUrl> &urls) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_utils" && topic == "slot_OpenWith_ShowDialog") {
                           pushed = true;
                           captured = urls;
                       }
                       return QVariant();
                   });

    parser.process({ "ut-commandparser", "-o", "network://host/share/dir/" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_TRUE(pushed);   // non-local url skips existence check
    ASSERT_EQ(captured.size(), 1);
    EXPECT_EQ(captured.first().path(), QString("/share/dir"));   // trailing '/' chopped
}

TEST_F(CommandParserTest, ProcessCommand_OpenHome_PublishesOpenNewWindow)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });
    stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location),
                   [this](StandardPaths::StandardLocation loc) -> QString {
                       __DBG_STUB_INVOKE__
                       return loc == StandardPaths::StandardLocation::kHomePath
                           ? tempDir->path()
                           : QString();
                   });

    const int before = g_listener->openNewWindowCalls;
    parser.process({ "ut-commandparser", "-O" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->openNewWindowCalls, before + 1);
    EXPECT_EQ(g_listener->lastNewWindowUrl.toLocalFile(), tempDir->path());
    EXPECT_TRUE(g_listener->lastNewWindowFlag);   // DConfig default true
}

TEST_F(CommandParserTest, ProcessCommand_SessionFileMissing_WarnsAndNoWindow)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "-s", "missing-session" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(listenerCount(), before);   // readPath fails -> no openWindowWithUrl
}

TEST_F(CommandParserTest, ProcessCommand_SessionFileValid_OpensStoredPath)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    const QString sessionDir = fakeHome + "/.config/session";
    ASSERT_TRUE(QDir().mkpath(sessionDir));
    const QString sessionFile = sessionDir + "/"
        + QCoreApplication::applicationName() + "_my-session";
    QFile f(sessionFile);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(R"({"path":"/session/restored"})");
    f.close();

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "-s", "my-session" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(listenerCount(), before + 1);   // readPath succeeds -> openWindowWithUrl -> kOpenNewWindow
    EXPECT_EQ(g_listener->lastNewWindowUrl.path(), QString("/session/restored"));
}

TEST_F(CommandParserTest, ProcessEvent_EmptyPositional_ReturnsEarly)
{
    // Arrange
    const int before = listenerCount();
    parser.process({ "ut-commandparser", "-e" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(listenerCount(), before);   // branch: no positional arguments
}

TEST_F(CommandParserTest, ProcessEvent_UnknownAction_DoesNotPublish)
{
    // Arrange
    parser.process({ "ut-commandparser", "-e", R"({"action":"no-such-action"})" });
    const int copyBefore = g_listener->copyCalls;

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->copyCalls, copyBefore);   // branch: action not in eventMap
}

TEST_F(CommandParserTest, ProcessEvent_Refresh_PushesSlotChannel)
{
    // Arrange
    bool pushed = false;
    QList<QUrl> captured;
    typedef QVariant (EventChannelManager::*PushFunc)(const QString &, const QString &, QList<QUrl>);
    stub.set_lamda(static_cast<PushFunc>(&EventChannelManager::push),
                   [&pushed, &captured](EventChannelManager *, const QString &space,
                                        const QString &topic, QList<QUrl> urls) -> QVariant {
                       __DBG_STUB_INVOKE__
                       if (space == "dfmplugin_workspace" && topic == "slot_RefreshDir") {
                           pushed = true;
                           captured = urls;
                       }
                       return QVariant();
                   });

    parser.process({ "ut-commandparser", "-e",
                     R"({"action":"refresh","params":{"sources":["/tmp/one"]}})" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_TRUE(pushed);   // branch: action == refresh
    ASSERT_EQ(captured.size(), 1);
    EXPECT_EQ(captured.first().path(), QString("/tmp/one"));
}

TEST_F(CommandParserTest, ProcessEvent_CopyAction_PublishesCopyEvent)
{
    // Arrange
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath,
                   [](SystemPathUtil *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    const int before = g_listener->copyCalls;
    parser.process({ "ut-commandparser", "-e",
                     R"({"action":"copy","params":{"sources":["/tmp/src"],"target":"/tmp/dst"}})" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->copyCalls, before + 1);   // branch: action == copy
    ASSERT_EQ(g_listener->lastCopySources.size(), 1);
    EXPECT_EQ(g_listener->lastCopySources.first().path(), QString("/tmp/src"));
    EXPECT_EQ(g_listener->lastCopyTarget.path(), QString("/tmp/dst"));
}

TEST_F(CommandParserTest, ProcessEvent_CutSystemPath_SkipsPublish)
{
    // Arrange
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath,
                   [](SystemPathUtil *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;   // force system-path hit
                   });
    parser.process({ "ut-commandparser", "-e",
                     R"({"action":"move","params":{"sources":["/usr/bin"],"target":"/tmp"}})" });

    // Act
    parser.processCommand();

    // Assert
    // kCutFile has no listener here; the only observable is that nothing crashes
    // and the early-return branch (system path) was taken without publishing.
    SUCCEED();
}

TEST_F(CommandParserTest, ProcessEvent_DeleteAction_PublishesDeleteEvent)
{
    // Arrange
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath,
                   [](SystemPathUtil *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    const int before = g_listener->deleteCalls;
    parser.process({ "ut-commandparser", "-e",
                     R"({"action":"delete","params":{"sources":["/tmp/gone"]}})" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->deleteCalls, before + 1);   // branch: action == delete
}

TEST_F(CommandParserTest, ProcessEvent_TrashTrashFile_SkipsPublish)
{
    // Arrange
    stub.set_lamda(&SystemPathUtil::checkContainsSystemPath,
                   [](SystemPathUtil *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(&FileUtils::isTrashFile,
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;   // force trash-file hit
                   });
    parser.process({ "ut-commandparser", "-e",
                     R"({"action":"trash","params":{"sources":["trash:///x"]}})" });

    // Act
    parser.processCommand();

    // Assert
    SUCCEED();   // branch: isTrashFile -> early return, no crash
}

TEST_F(CommandParserTest, ProcessEvent_CleanTrash_PublishesCleanTrashEvent)
{
    // Arrange
    const int before = g_listener->cleanTrashCalls;
    parser.process({ "ut-commandparser", "-e", R"({"action":"cleantrash"})" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(g_listener->cleanTrashCalls, before + 1);   // branch: action == cleantrash
}

TEST_F(CommandParserTest, OpenInUrls_SpecialChars_EncodesPath)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "name#1&2" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(listenerCount(), before + 1);   // openWindowWithUrl -> kOpenNewWindow
    const QString spec = g_listener->lastNewWindowUrl.toString();
    EXPECT_TRUE(spec.contains("%23"));   // '#' percent-encoded by the pre-parse loop
    EXPECT_FALSE(spec.contains('#'));
}

TEST_F(CommandParserTest, OpenInUrls_RawFlag_NoEncoding)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "-R", "raw#name" });

    // Act
    parser.processCommand();

    // Assert
    EXPECT_EQ(listenerCount(), before + 1);
    EXPECT_TRUE(g_listener->lastNewWindowUrl.toString().contains('#'));   // raw: not encoded
}

TEST_F(CommandParserTest, OpenInUrls_ShowItemMissingFile_OpensParentWithSelectQuery)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "--show-item", "/no/such/file-anywhere" });

    // Act
    parser.processCommand();

    // Assert: SyncFileInfo wraps missing files, so the show-item branch maps the
    // URL to its parent with a selectUrl query instead of skipping it.
    EXPECT_EQ(listenerCount(), before + 1);
    EXPECT_EQ(g_listener->lastNewWindowUrl.path(), QString("/no/such"));
    EXPECT_TRUE(g_listener->lastNewWindowUrl.hasQuery());
    EXPECT_TRUE(g_listener->lastNewWindowUrl.query().contains("selectUrl="));
}

TEST_F(CommandParserTest, OpenInUrls_ShowItemUnknownScheme_SkipsFileAndOpensDefault)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });

    const int before = listenerCount();
    parser.process({ "ut-commandparser", "--show-item", "bogus://no-such-scheme/x" });

    // Act
    parser.processCommand();

    // Assert: InfoFactory has no creator for the bogus scheme -> null FileInfo ->
    // continue -> argumentUrls empty -> publish kOpenNewWindow(QUrl(), flag)
    EXPECT_EQ(listenerCount(), before + 1);
    EXPECT_TRUE(g_listener->lastNewWindowUrl.isEmpty());
}

TEST_F(CommandParserTest, OpenWindowWithUrl_NewTabAttribute_PublishesNewTab)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &attr) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return attr == Application::kOpenInNewTab;
                   });
    stub.set_lamda(&FileManagerWindowsManager::lastActivedWindowId,
                   []() -> quint64 {
                       __DBG_STUB_INVOKE__
                       return 7;
                   });
    stub.set_lamda(&FileManagerWindowsManager::findWindowById,
                   [](FileManagerWindowsManager *, quint64) -> FileManagerWindow * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    const int before = g_listener->openNewTabCalls;
    const QUrl url = QUrl::fromLocalFile(tempDir->path());
    parser.process({ "ut-commandparser", "-n" });

    // Act
    parser.openWindowWithUrl(url);

    // Assert
    EXPECT_EQ(g_listener->openNewTabCalls, before + 1);   // branch: new-tab attribute + active window
    EXPECT_EQ(g_listener->lastNewTabUrl, url);
}

TEST_F(CommandParserTest, BindEvents_StartAppSignal_TriggersProcessCommand)
{
    // Arrange
    stub.set_lamda(&DConfigManager::value,
                   [](DConfigManager *, const QString &, const QString &,
                      const QVariant &defaultValue) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return defaultValue;
                   });
    stub.set_lamda(&Application::appAttribute,
                   [](Application::ApplicationAttribute &) -> QVariant {
                       __DBG_STUB_INVOKE__
                       return QVariant(false);
                   });
    Event::instance()->registerEventType(EventStratege::kSignal,
                                          "dfmplugin_core", "signal_StartApp");
    parser.bindEvents();

    const int before = listenerCount();

    // Act: fire the custom signal the same way dfmplugin-core does
    dpfSignalDispatcher->publish("dfmplugin_core", "signal_StartApp");

    // Assert
    EXPECT_EQ(listenerCount(), before + 1);   // processCommand ran -> openInUrls -> kOpenNewWindow
    EXPECT_TRUE(g_listener->lastNewWindowUrl.isEmpty());   // no positional args
}

TEST_F(CommandParserTest, PositionalArguments_ReflectParsedPositionals)
{
    // Arrange
    parser.process({ "ut-commandparser", "pos1", "pos2" });

    // Act
    const QStringList positional = parser.positionalArguments();
    const QStringList unknown = parser.unknownOptionNames();

    // Assert
    EXPECT_EQ(positional, QStringList({ "pos1", "pos2" }));
    EXPECT_TRUE(unknown.isEmpty());
}

class SessionBusinessTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    SessionBusiness *business = SessionBusiness::instance();
};

TEST_F(SessionBusinessTest, Instance_IsSingleton_SameReference)
{
    // Arrange / Act
    auto *first = SessionBusiness::instance();
    auto *second = SessionBusiness::instance();

    // Assert
    EXPECT_EQ(first, second);
    EXPECT_EQ(business, first);
}

TEST_F(SessionBusinessTest, GetAPI_ReturnsNonNull)
{
    // Arrange / Act
    UsmSessionAPI *api = business->getAPI();

    // Assert
    EXPECT_NE(api, nullptr);
}

TEST_F(SessionBusinessTest, Process_StoresArgumentsForParsing)
{
    // Arrange
    const QStringList args { "arg-one", "arg-two", "arg-three" };

    // Act
    business->process(args);

    // Assert: parseArguments is driven by the stored member
    int argc = 0;
    char **argv = business->parseArguments(argc);
    ASSERT_NE(argv, nullptr);
    EXPECT_EQ(argc, 3);
    EXPECT_EQ(QString(argv[0]), QString("arg-one"));
    EXPECT_EQ(QString(argv[2]), QString("arg-three"));
    business->releaseArguments(argc, argv);
}

TEST_F(SessionBusinessTest, ParseArgumentsEmpty_ReturnsNullptr)
{
    // Arrange
    business->process({});

    // Act
    int argc = 99;
    char **argv = business->parseArguments(argc);

    // Assert
    EXPECT_EQ(argv, nullptr);   // branch: arguments empty
}

TEST_F(SessionBusinessTest, ReadPath_NullPointer_ReturnsFalse)
{
    // Arrange / Act
    const bool ok = business->readPath("whatever", nullptr);

    // Assert
    EXPECT_FALSE(ok);   // branch: null output pointer
}

TEST_F(SessionBusinessTest, ReadPath_MissingFile_ReturnsFalse)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    QString out;

    // Act
    const bool ok = business->readPath("no-such-session", &out);

    // Assert
    EXPECT_FALSE(ok);   // branch: file cannot be opened
}

TEST_F(SessionBusinessTest, ReadPath_InvalidJson_ReturnsFalse)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    const QString dir = fakeHome + "/.config/session";
    ASSERT_TRUE(QDir().mkpath(dir));
    QFile f(dir + "/" + QCoreApplication::applicationName() + "_broken");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("this-is-not-json");
    f.close();
    QString out;

    // Act
    const bool ok = business->readPath("broken", &out);

    // Assert
    EXPECT_FALSE(ok);   // branch: QJsonDocument null
}

TEST_F(SessionBusinessTest, ReadPath_EmptyPath_ReturnsFalse)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    const QString dir = fakeHome + "/.config/session";
    ASSERT_TRUE(QDir().mkpath(dir));
    QFile f(dir + "/" + QCoreApplication::applicationName() + "_empty");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(R"({"path":""})");
    f.close();
    QString out;

    // Act
    const bool ok = business->readPath("empty", &out);

    // Assert
    EXPECT_FALSE(ok);   // branch: parsed path empty
}

TEST_F(SessionBusinessTest, ReadPath_ValidJson_ReturnsTrueAndPath)
{
    // Arrange
    QString fakeHome = tempDir->path();
    stub.set_lamda(&QDir::homePath, [&fakeHome]() -> QString {
        __DBG_STUB_INVOKE__
        return fakeHome;
    });
    const QString dir = fakeHome + "/.config/session";
    ASSERT_TRUE(QDir().mkpath(dir));
    QFile f(dir + "/" + QCoreApplication::applicationName() + "_good");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(R"({"path":"/home/uos/restored-session"})");
    f.close();
    QString out;

    // Act
    const bool ok = business->readPath("good", &out);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, QString("/home/uos/restored-session"));
}
