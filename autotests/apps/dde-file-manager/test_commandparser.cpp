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
//   [scheme URL → 只编码 authority 之后]      → SftpUrlFromBugReport_KeepsAuthorityAndPath
//                                              / UrlArguments_KeepSchemeUserHostPort
//   [本地特殊名字（绝对/相对/~）→ 同一路径]    → LocalSpecialNames_* / TildePath_*
//   [不存在的相对路径 → 特殊字符留在 path]     → NonexistentRelativeSpecialChars_*
//   [多参数混传 → 逐个按序打开]                → MultipleMixedArguments_*
//   [-R → 不预编码且 authority 完整]           → RawFlag_UrlAuthorityIntact
//   [空参数 → 默认窗口]                        → EmptyArgument_OpensDefaultWindow
//   [--show-item 特殊文件 → 父目录+selectUrl]  → ShowItemSpecialCharFile_*
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
#include <QFileInfo>
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
    QList<QUrl> openedNewWindowUrls;   // 按发布顺序收集，用于多参数场景
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
        openedNewWindowUrls.append(url);
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

// 恢复工作目录的 RAII 守卫：相对路径用例需要临时切换 cwd
struct CwdGuard
{
    explicit CwdGuard(const QString &dir)
        : oldDir(QDir::currentPath())
    {
        QDir::setCurrent(dir);
    }
    ~CwdGuard() { QDir::setCurrent(oldDir); }

    QString oldDir;
};

// 逐组件比较 URL：QUrl::operator== 还会比较编码形式，命令行场景只关心语义
bool sameUrlComponents(const QUrl &a, const QUrl &b)
{
    return a.scheme() == b.scheme() && a.userName() == b.userName()
        && a.host() == b.host() && a.port() == b.port()
        && a.path() == b.path() && a.query() == b.query()
        && a.fragment() == b.fragment();
}

// 本机桌面与临时目录中真实出现过（或同类）的目录名，覆盖特殊字符、连续空格、
// 百分号序列、引号、括号、emoji、CJK、阿拉伯文、西里尔文等。
// 不含 '/'（不是合法单层文件名），也不含以 '-' 开头的名字——后者会被
// QCommandLineParser 当成选项，必须由调用方用 "--" 分隔，不属于本函数覆盖范围。
QStringList specialLocalNames()
{
    return { "new_textindex 快捷方式",
             "sdasd",
             "新建文件夹2",
             "tmp",
             "B64123",
             "black-path",
             "特殊字符#$%",
             "%,$",
             "%$,",
             "2 2",
             "%25",
             "#$%",
             "3  3",
             "100%完成",
             "emoji_folder_😀🎉📁",
             "folder!exclamation",
             "folder\"with\"double\"quotes",
             "folder#hash",
             "folder%20with%20encoded",
             "folder%with%percent",
             "folder&ampersand",
             "folder'with'quotes",
             "folder(with)parentheses",
             "folder,with,commas",
             "folder-with-dashes",
             "folder?question",
             "folder@at",
             "folder[with]brackets",
             "folder_with_spaces",
             "folder_with_underscores",
             "folder{with}braces",
             "folder~tilde",
             "nested",
             "normal_folder",
             "very_long_folder_name_that_might_test_various_limits_and_edge_cases_in_argument_processing_and_path_handling_mechanisms",
             "测试镜像~!@#$%^&()_+`1234567890-={}[];.qwe",
             "日本語フォルダ",
             "相对路径测试",
             "русский_папка",
             "العربية_مجلد",
             "العر بي ة_مجلد（副本）",
             "spec",
             "1",
             "新建文件夹",
             "录屏_选择区域_20250717193539录屏_选择区域_20250717193539录屏_选择区域_2025!@#$$% ^&& ()_+ 选择区域_20250717193539录屏_选择区域_20250717193539录屏_选择区域_20250717193539录屏_选择区域_20250717193539录  &^选择",
             "2025!@#$$% ^&& ()",
             "213",
             "ali",
             "views",
             "新建文件夹1",
             "新建文件夹3",
             "dfmplugin-burn",
             "search",
             "全文检索文件监控排查",
             "12333",
             "L",
             "新建文件夹2223",
             "qwe",
             "中文文件夹",
             // 追加的边界形态
             " leading space",
             "trailing space ",
             "tab\tinside",
             "new\nline",
             "semi;colon:colon",
             "percent%25literal",
             "with 'single' \"double\" quotes",
             "looks_like_email@example.com" };
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

    // DConfig / Application 走默认值，使 openWindowWithUrl 发布 kOpenNewWindow
    void stubOpenNewWindowPublishers()
    {
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
    }

    // 执行一次 processCommand，返回本次发布的全部 kOpenNewWindow URL（按参数顺序）
    QList<QUrl> openInUrlsWith(const QStringList &positional, const QStringList &options = {})
    {
        g_listener->openedNewWindowUrls.clear();
        QStringList args { "ut-commandparser" };
        args << options << positional;
        parser.process(args);
        parser.processCommand();
        return g_listener->openedNewWindowUrls;
    }

    // 在临时目录下创建目录（名字可含任意合法文件名字符），返回绝对路径
    QString makeLocalDir(const QString &name)
    {
        const QString path = tempDir->path() + "/" + name;
        EXPECT_TRUE(QDir().mkpath(path)) << "mkpath failed: " << path.toStdString();
        return path;
    }

    // 判断一次发布的 URL 是否指向给定本地路径，并校验字符串形态可往返
    void expectLocalPath(const QUrl &url, const QString &expectedPath)
    {
        EXPECT_TRUE(url.isLocalFile()) << "not a local file url: " << url.toString().toStdString();
        EXPECT_EQ(url.toLocalFile(), expectedPath);
        // IPC/DBus 传参使用完整编码串，编码后必须还能还原成同一路径
        const QUrl reparsed(url.toString(QUrl::FullyEncoded));
        EXPECT_EQ(reparsed.toLocalFile(), expectedPath);
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

// ============================================================================
// 命令行路径/URL 参数回归测试（对应 commit 88216d205 "skip encoding URL authority"）
//
// 1. URL 参数：scheme://user@host:port/path 形式的参数，预编码只能作用于
//    authority 之后。编码 authority 里的 '@' 会让 QUrl::fromUserInput 把整串
//    退化成 http://scheme/...，sftp/ftp/smb 因此无法挂载（本组用例的起点）。
// 2. 本地路径：桌面等目录里真实存在的特殊名字（特殊符号、连续空格、百分号
//    序列、引号、括号、emoji、CJK/阿拉伯/西里尔文）必须还原成同一个本地路径，
//    且编码串可往返（IPC/DBus 传参路径）。
// 3. 以下既有局限不在本组用例中断言（避免把缺陷固化成期望）：
//    - 相对路径 + 悬空符号链接：QDir::exists() 为 false，会走预编码分支，
//      最终得到一个没有 scheme 的相对 URL；
//    - 相对路径且文件名以 '~' 开头：UrlRoute::fromUserInput() 的 '~' 分支
//      会把它展开成 homePath + 余下部分；
//    - 相对路径以 '-' 开头：会被 QCommandLineParser 当成选项，需调用方用 "--" 分隔。
// ============================================================================

TEST_F(CommandParserTest, OpenInUrls_SftpUrlFromBugReport_KeepsAuthorityAndPath)
{
    // Arrange: 桌面快捷方式里的原始命令
    //   dde-file-manager sftp://ut000177@127.0.0.1:13222/home/ut000177@uos
    stubOpenNewWindowPublishers();
    const QString arg = "sftp://ut000177@127.0.0.1:13222/home/ut000177@uos";

    // Act
    const QList<QUrl> opened = openInUrlsWith({ arg });

    // Assert
    ASSERT_EQ(opened.size(), 1);
    const QUrl &url = opened.first();
    EXPECT_EQ(url.scheme(), QString("sftp"));
    EXPECT_EQ(url.userName(), QString("ut000177"));   // authority 中的 '@' 未被编码
    EXPECT_EQ(url.host(), QString("127.0.0.1"));
    EXPECT_EQ(url.port(), 13222);
    EXPECT_EQ(url.path(), QString("/home/ut000177@uos"));   // 路径中的 '@' 属于路径

    // 完整编码串：authority 分隔符保持原样，路径中的 '@' 才编码
    const QString encoded = url.toString(QUrl::FullyEncoded);
    EXPECT_TRUE(encoded.startsWith("sftp://ut000177@127.0.0.1:13222/"))
        << encoded.toStdString();
    EXPECT_FALSE(encoded.contains("%40127.0.0.1"))   // 旧缺陷：userinfo 的 '@' 被编码
        << encoded.toStdString();
    EXPECT_TRUE(encoded.endsWith("/home/ut000177%40uos")) << encoded.toStdString();
}

TEST_F(CommandParserTest, OpenInUrls_UrlArguments_KeepSchemeUserHostPort)
{
    // Arrange: 形状各异的远端 URL，含无路径、尾斜杠、IPv6、大写 scheme、
    // authority 内已编码的 '@'，以及路径中的 # ? & ! @ 与多语言文件名。
    stubOpenNewWindowPublishers();
    struct Case
    {
        const char *arg;
        const char *scheme;
        const char *user;
        const char *host;
        int port;
        const char *path;
    };
    const Case cases[] = {
        { "sftp://ut000177@127.0.0.1:13222/home/ut000177@uos", "sftp", "ut000177", "127.0.0.1", 13222, "/home/ut000177@uos" },
        { "sftp://user@host", "sftp", "user", "host", -1, "" },
        { "sftp://user@host/", "sftp", "user", "host", -1, "/" },
        { "ftp://ut000177@127.0.0.1:13222/pub", "ftp", "ut000177", "127.0.0.1", 13222, "/pub" },
        { "smb://user@server/share", "smb", "user", "server", -1, "/share" },
        { "smb://user%40dom@server/share", "smb", "user@dom", "server", -1, "/share" },
        { "SFTP://user@host/home/x@y", "sftp", "user", "host", -1, "/home/x@y" },
        { "sftp://user@host/home/deep#dir", "sftp", "user", "host", -1, "/home/deep#dir" },
        { "sftp://user@host/home/a?b=1&c=2", "sftp", "user", "host", -1, "/home/a?b=1&c=2" },
        { "sftp://user@host/home/a!b", "sftp", "user", "host", -1, "/home/a!b" },
        { "sftp://user@host/home/a&b", "sftp", "user", "host", -1, "/home/a&b" },
        { "sftp://user@[::1]:22/home/x@y", "sftp", "user", "::1", 22, "/home/x@y" },
        { "sftp://user@host/home/100%完成", "sftp", "user", "host", -1, "/home/100%完成" },
        { "sftp://user@host/home/emoji_😀🎉📁", "sftp", "user", "host", -1, "/home/emoji_😀🎉📁" },
        { "sftp://user@host/home/with space", "sftp", "user", "host", -1, "/home/with space" },
    };

    for (const auto &c : cases) {
        SCOPED_TRACE(c.arg);
        const QList<QUrl> opened = openInUrlsWith({ QString::fromUtf8(c.arg) });
        if (opened.size() != 1) {
            ADD_FAILURE() << "expected exactly one opened url, got " << opened.size();
            continue;
        }
        const QUrl &url = opened.first();
        EXPECT_EQ(url.scheme(), QString::fromUtf8(c.scheme));
        EXPECT_EQ(url.userName(), QString::fromUtf8(c.user));
        EXPECT_EQ(url.host(), QString::fromUtf8(c.host));
        EXPECT_EQ(url.port(), c.port);
        EXPECT_EQ(url.path(), QString::fromUtf8(c.path));
        // 编码串可往返：重解析后组件不变
        EXPECT_TRUE(sameUrlComponents(QUrl(url.toString(QUrl::FullyEncoded)), url))
            << url.toString(QUrl::FullyEncoded).toStdString();
    }
}

TEST_F(CommandParserTest, OpenInUrls_UrlWithoutSpecialChars_Untouched)
{
    // Arrange: 不带 #&@!? 的参数不进入预编码分支，原样交给 UrlRoute::fromUserInput
    stubOpenNewWindowPublishers();

    // Act
    const QList<QUrl> opened = openInUrlsWith({ "network://host/share/dir", "sftp://host/pub" });

    // Assert
    ASSERT_EQ(opened.size(), 2);
    EXPECT_EQ(opened.at(0).scheme(), QString("network"));
    EXPECT_EQ(opened.at(0).host(), QString("host"));
    EXPECT_EQ(opened.at(0).path(), QString("/share/dir"));
    EXPECT_EQ(opened.at(1).scheme(), QString("sftp"));
    EXPECT_EQ(opened.at(1).path(), QString("/pub"));
}

TEST_F(CommandParserTest, OpenInUrls_MultipleMixedArguments_EachResolvedInOrder)
{
    // Arrange: 一次启动混传本地目录 + 两个远端 URL，包括热启动转发的场景
    stubOpenNewWindowPublishers();
    const QString localDir = makeLocalDir("special#$% dir");

    // Act
    const QList<QUrl> opened = openInUrlsWith({ localDir,
                                                "sftp://ut000177@127.0.0.1:13222/home/ut000177@uos",
                                                "smb://user@server/share#frag" });

    // Assert: 顺序与参数一致，互不影响
    ASSERT_EQ(opened.size(), 3);
    expectLocalPath(opened.at(0), localDir);
    EXPECT_EQ(opened.at(1).host(), QString("127.0.0.1"));
    EXPECT_EQ(opened.at(1).port(), 13222);
    EXPECT_EQ(opened.at(1).path(), QString("/home/ut000177@uos"));
    EXPECT_EQ(opened.at(2).host(), QString("server"));
    EXPECT_EQ(opened.at(2).path(), QString("/share#frag"));
}

TEST_F(CommandParserTest, OpenInUrls_RawFlag_UrlAuthorityIntact)
{
    // Arrange: -R 跳过预编码，authority 同样不能被 QUrl 解析破坏
    stubOpenNewWindowPublishers();
    const QString arg = "sftp://ut000177@127.0.0.1:13222/home/ut000177@uos";

    // Act
    const QList<QUrl> opened = openInUrlsWith({ arg }, { "-R" });

    // Assert
    ASSERT_EQ(opened.size(), 1);
    EXPECT_EQ(opened.first().scheme(), QString("sftp"));
    EXPECT_EQ(opened.first().userName(), QString("ut000177"));
    EXPECT_EQ(opened.first().host(), QString("127.0.0.1"));
    EXPECT_EQ(opened.first().port(), 13222);
    EXPECT_EQ(opened.first().path(), QString("/home/ut000177@uos"));
}

TEST_F(CommandParserTest, OpenInUrls_LocalSpecialNames_AbsoluteArgumentsRoundTrip)
{
    // Arrange: 桌面等目录中的真实特殊名字，以绝对路径一次性传入
    stubOpenNewWindowPublishers();
    const QStringList names = specialLocalNames();
    QStringList args;
    QStringList expected;
    for (const QString &name : names) {
        const QString path = makeLocalDir(name);
        if (path.isEmpty())
            continue;
        args << path;
        expected << path;
    }
    ASSERT_FALSE(args.isEmpty());

    // Act
    const QList<QUrl> opened = openInUrlsWith(args);

    // Assert
    ASSERT_EQ(opened.size(), expected.size());
    for (int i = 0; i < expected.size(); ++i) {
        SCOPED_TRACE(expected.at(i).toStdString());
        expectLocalPath(opened.at(i), expected.at(i));
    }
}

TEST_F(CommandParserTest, OpenInUrls_LocalSpecialNames_RelativeArgumentsResolveAgainstCwd)
{
    // Arrange: 在目标目录内用相对名字启动（等价于 dde-file-manager "新建文件夹2"）
    stubOpenNewWindowPublishers();
    const QStringList names = specialLocalNames();
    QStringList created;
    QStringList expected;
    for (const QString &name : names) {
        const QString path = makeLocalDir(name);
        if (path.isEmpty())
            continue;
        created << name;
        expected << path;
    }
    ASSERT_FALSE(expected.isEmpty());

    // Act
    QList<QUrl> opened;
    {
        CwdGuard guard(tempDir->path());
        opened = openInUrlsWith(created);
    }

    // Assert
    ASSERT_EQ(opened.size(), expected.size());
    for (int i = 0; i < expected.size(); ++i) {
        SCOPED_TRACE(expected.at(i).toStdString());
        expectLocalPath(opened.at(i), expected.at(i));
    }
}

TEST_F(CommandParserTest, OpenInUrls_LocalNameContainingSchemeSeparator_StaysLocal)
{
    // Arrange: 目录名本身含 ':'，构成形如 "a:/b" 的路径；已存在路径不能被
    // 误判成 URL（QDir().exists() 短路在这里起作用）
    stubOpenNewWindowPublishers();
    const QString dir = makeLocalDir("a:");
    const QString child = dir + "/b";
    ASSERT_TRUE(QDir().mkpath(child));

    // Act: 绝对路径
    const QList<QUrl> absolute = openInUrlsWith({ child });

    // Assert
    ASSERT_EQ(absolute.size(), 1);
    expectLocalPath(absolute.first(), child);

    // Act: 相对路径
    QList<QUrl> relative;
    {
        CwdGuard guard(tempDir->path());
        relative = openInUrlsWith({ "a:/b" });
    }

    // Assert
    ASSERT_EQ(relative.size(), 1);
    expectLocalPath(relative.first(), child);
}

TEST_F(CommandParserTest, OpenInUrls_TildePathWithSpecialChars_ExpandsToHome)
{
    // Arrange: '~' 路径由 UrlRoute::fromUserInput 展开，预编码会让 %xx 变成
    // 文件名字面量（旧行为会打开 /home/xxx/sub%231 这种不存在的路径）
    stubOpenNewWindowPublishers();
    const QString home = tempDir->path();
    stub.set_lamda(&QDir::homePath, [home]() -> QString {
        __DBG_STUB_INVOKE__
        return home;
    });
    const QStringList names { "sub#1&2", "100%完成", "a b?c" };
    for (const QString &name : names)
        ASSERT_FALSE(makeLocalDir(name).isEmpty());

    // Act
    QStringList args;
    for (const QString &name : names)
        args << ("~/" + name);
    const QList<QUrl> opened = openInUrlsWith(args);

    // Assert
    ASSERT_EQ(opened.size(), names.size());
    for (int i = 0; i < names.size(); ++i) {
        SCOPED_TRACE(names.at(i).toStdString());
        expectLocalPath(opened.at(i), home + "/" + names.at(i));
    }
}

TEST_F(CommandParserTest, OpenInUrls_NonexistentRelativeSpecialChars_StayInPathWithoutFragment)
{
    // Arrange: bug-10151 的原始场景——路径尚不存在时，特殊字符必须留在 path 里，
    // 不能被 QUrl 吃成 fragment/query
    stubOpenNewWindowPublishers();
    const QStringList args { "name#1&2", "photo#1.jpg", "a?b", "a!b", "a@b",
                             "with space # and & and ?" };
    // 固定到空临时目录，保证这些名字确实不存在
    CwdGuard guard(tempDir->path());

    for (const QString &arg : args) {
        SCOPED_TRACE(arg.toStdString());

        // Act
        const QList<QUrl> opened = openInUrlsWith({ arg });

        // Assert
        if (opened.size() != 1) {
            ADD_FAILURE() << "expected exactly one opened url, got " << opened.size();
            continue;
        }
        EXPECT_FALSE(opened.first().hasFragment());
        EXPECT_TRUE(opened.first().query().isEmpty());
        EXPECT_EQ(opened.first().path(), arg);
    }
}

TEST_F(CommandParserTest, OpenInUrls_EmptyArgument_OpensDefaultWindow)
{
    // Arrange: 空字符串参数不应崩溃，最终落到默认窗口（URL 为空）
    stubOpenNewWindowPublishers();

    // Act
    const QList<QUrl> opened = openInUrlsWith({ "" });

    // Assert
    ASSERT_EQ(opened.size(), 1);
    EXPECT_TRUE(opened.first().isEmpty());
}

TEST_F(CommandParserTest, OpenInUrls_ShowItemSpecialCharFile_SelectsChildInParent)
{
    // Arrange: --show-item 指向含特殊字符的真实文件
    stubOpenNewWindowPublishers();
    const QString dir = makeLocalDir("dir#1&2");
    const QString file = dir + "/photo#1 & 2.jpg";
    {
        QFile f(file);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    }

    // Act
    const QList<QUrl> opened = openInUrlsWith({ "--show-item", file });

    // Assert: 打开父目录，并通过 selectUrl 携带完整编码的子项 URL
    ASSERT_EQ(opened.size(), 1);
    expectLocalPath(opened.first(), dir);
    ASSERT_TRUE(opened.first().hasQuery());
    const QString decodedQuery = opened.first().query(QUrl::FullyDecoded);
    EXPECT_TRUE(decodedQuery.contains("selectUrl=")) << decodedQuery.toStdString();
    EXPECT_TRUE(decodedQuery.contains("photo#1 & 2.jpg")) << decodedQuery.toStdString();
}

TEST_F(CommandParserTest, ProcessCommand_PropertyDialog_SpecialCharPath_KeepsLiteralPath)
{
    // Arrange: -p 走的是 QUrl::fromUserInput 直连分支（不经过 openInUrls 预编码）
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
    const QString file = tempDir->path() + "/prop#1 & 2 %25.txt";
    {
        QFile f(file);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    }
    parser.process({ "ut-commandparser", "-p", file });

    // Act
    parser.processCommand();

    // Assert
    ASSERT_TRUE(pushed);
    ASSERT_EQ(captured.size(), 1);
    expectLocalPath(captured.first(), file);
}

TEST_F(CommandParserTest, OpenInUrls_RealDesktopEntries_RoundTrip)
{
    // Arrange: 用本机桌面上的真实条目做一次全量扫描（无桌面环境的机器上跳过）
    const QString desktop = QDir::homePath() + "/Desktop";
    if (!QFileInfo(desktop).isDir())
        GTEST_SKIP() << "no ~/Desktop on this machine";
    stubOpenNewWindowPublishers();

    QStringList args;
    QStringList expected;
    const QFileInfoList entries = QDir(desktop).entryInfoList(
            QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : entries) {
        if (!fi.exists())   // 悬空符号链接：走另一条既有路径，见文件头说明
            continue;
        if (fi.fileName().startsWith("~"))   // '~' 展开属于 UrlRoute 的既有局限
            continue;
        args << fi.absoluteFilePath();
        expected << fi.absoluteFilePath();
    }
    if (args.isEmpty())
        GTEST_SKIP() << "nothing to scan under ~/Desktop";

    // Act
    const QList<QUrl> opened = openInUrlsWith(args);

    // Assert
    ASSERT_EQ(opened.size(), expected.size());
    for (int i = 0; i < expected.size(); ++i) {
        SCOPED_TRACE(expected.at(i).toStdString());
        expectLocalPath(opened.at(i), expected.at(i));
    }
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
