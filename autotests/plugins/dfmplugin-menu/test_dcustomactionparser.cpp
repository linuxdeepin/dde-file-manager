// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"
#include "extendmenuscene/extendmenu/dcustomactiondefine.h"

#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

using namespace dfmplugin_menu;
using namespace DCustomActionDefines;

static void writeConf(const QString &path, const QString &content)
{
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    ASSERT_TRUE(file.write(content.toLocal8Bit()) > 0);
    file.close();
}

// 一份最小但完整有效的自定义菜单配置
static QString minimalValidConf()
{
    return QString("[Menu Entry]\n"
                   "Version=1.0\n"
                   "Actions=act1\n"
                   "Comment=minimal valid conf\n"
                   "Sign=test-sign\n"
                   "\n"
                   "[Menu Action act1]\n"
                   "Name=Test Action\n"
                   "Exec=/bin/echo %f\n"
                   "X-DDE-FileManager-MenuTypes=SingleFile\n");
}

class UT_RegisterCustomFormat : public testing::Test
{
protected:
    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_RegisterCustomFormat, Instance_ReturnsSameSingleton)
{
    RegisterCustomFormat &first = RegisterCustomFormat::instance();
    RegisterCustomFormat &second = RegisterCustomFormat::instance();
    EXPECT_EQ(&first, &second);
}

TEST_F(UT_RegisterCustomFormat, CustomFormat_IsRegisteredAndStable)
{
    auto format = RegisterCustomFormat::instance().customFormat();
    EXPECT_NE(format, QSettings::InvalidFormat);
    EXPECT_EQ(format, RegisterCustomFormat::instance().customFormat());
}

TEST_F(UT_RegisterCustomFormat, ReadConf_ValidContent_ReturnsTrueAndParsesMap)
{
    QByteArray content = "# this is a comment\n"
                         "[Menu Entry]\n"
                         "Version=1.0\n"
                         "Comment=hello world\n"
                         "\n"
                         "[Menu Action act1]\n"
                         "Name=Test\n"
                         "Exec=/bin/echo\n";
    QBuffer buffer(&content);
    ASSERT_TRUE(buffer.open(QIODevice::ReadOnly));

    QSettings::SettingsMap map;
    bool ret = RegisterCustomFormat::readConf(buffer, map);

    EXPECT_TRUE(ret);
    EXPECT_EQ(map.value("Menu Entry/Version").toString(), "1.0");
    EXPECT_EQ(map.value("Menu Entry/Comment").toString(), "hello world");
    EXPECT_EQ(map.value("Menu Action act1/Name").toString(), "Test");
    EXPECT_EQ(map.value("Menu Action act1/Exec").toString(), "/bin/echo");
    // 注释行不会被解析
    EXPECT_FALSE(map.contains(QString("Menu Entry/# this is a comment")));
}

TEST_F(UT_RegisterCustomFormat, ReadConf_NoMenuEntryPrefix_ReturnsFalse)
{
    // 分组前出现的 key=value 挂在空 section 下
    QByteArray content = "Version=1.0\n"
                         "[Some Group]\n"
                         "Key=Value\n";
    QBuffer buffer(&content);
    ASSERT_TRUE(buffer.open(QIODevice::ReadOnly));

    QSettings::SettingsMap map;
    bool ret = RegisterCustomFormat::readConf(buffer, map);

    EXPECT_FALSE(ret);
    EXPECT_EQ(map.value("/Version").toString(), "1.0");
    EXPECT_EQ(map.value("Some Group/Key").toString(), "Value");
}

TEST_F(UT_RegisterCustomFormat, ReadConf_EmptyKeyLine_Skipped)
{
    QByteArray content = "[Menu Entry]\n"
                         "=value only\n"
                         "Version=2.0\n";
    QBuffer buffer(&content);
    ASSERT_TRUE(buffer.open(QIODevice::ReadOnly));

    QSettings::SettingsMap map;
    bool ret = RegisterCustomFormat::readConf(buffer, map);

    EXPECT_TRUE(ret);
    EXPECT_FALSE(map.contains(QString("Menu Entry/")));
    EXPECT_EQ(map.value("Menu Entry/Version").toString(), "2.0");
}

TEST_F(UT_RegisterCustomFormat, WriteConf_AlwaysReturnsTrue)
{
    QBuffer buffer;
    ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
    QSettings::SettingsMap map;
    map["Menu Entry/Version"] = QVariant("1.0");
    EXPECT_TRUE(RegisterCustomFormat::writeConf(buffer, map));
}

class UT_DCustomActionParser : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        parser = new DCustomActionParser();
    }

    virtual void TearDown() override
    {
        delete parser;
        parser = nullptr;
        stub.clear();
    }

    DCustomActionEntry parseSingleEntry(const QString &conf)
    {
        writeConf(tempDir.filePath("single.conf"), conf);
        parser->menuPaths = QStringList { tempDir.path() };
        parser->refresh();
        QList<DCustomActionEntry> entries = parser->getActionFiles(true);
        return entries.isEmpty() ? DCustomActionEntry() : entries.first();
    }

protected:
    DCustomActionParser *parser { nullptr };
    QTemporaryDir tempDir;
    stub_ext::StubExt stub;
};

// ---- 构造 / 析构 ----

TEST_F(UT_DCustomActionParser, ConstructorAndDestructor_NoCrash)
{
    // SetUp 中已完成构造，这里再构造一个带 parent 的实例覆盖该分支
    DCustomActionParser *child = new DCustomActionParser(parser);
    EXPECT_NE(child, nullptr);
    delete child;
}

// ---- loadDir ----

TEST_F(UT_DCustomActionParser, LoadDir_EmptyPathList_ReturnsFalse)
{
    EXPECT_FALSE(parser->loadDir(QStringList()));
}

TEST_F(UT_DCustomActionParser, LoadDir_NonExistentDir_ReturnsTrueWithNoEntries)
{
    EXPECT_TRUE(parser->loadDir(QStringList { "/tmp/ut-menu-not-exist-dir-xyz" }));
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, LoadDir_ValidConf_LoadsEntry)
{
    writeConf(tempDir.filePath("valid.conf"), minimalValidConf());
    EXPECT_TRUE(parser->loadDir(QStringList { tempDir.path() }));

    auto entries = parser->getActionFiles(true);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().data().name(), "Test Action");
    EXPECT_EQ(entries.first().version(), "1.0");
    EXPECT_EQ(entries.first().comment(), "minimal valid conf");
}

TEST_F(UT_DCustomActionParser, LoadDir_SameConfNameInDifferentDirs_Deduplicated)
{
    QTemporaryDir dir1, dir2;
    ASSERT_TRUE(dir1.isValid());
    ASSERT_TRUE(dir2.isValid());

    QString conf1 = minimalValidConf();
    QString conf2 = QString("[Menu Entry]\n"
                            "Version=1.0\n"
                            "\n"
                            "[Menu Action act1]\n"
                            "Name=Second Dir Action\n"
                            "Exec=/bin/echo %f\n"
                            "X-DDE-FileManager-MenuTypes=SingleFile\n");
    writeConf(dir1.filePath("dupe.conf"), conf1);
    writeConf(dir2.filePath("dupe.conf"), conf2);

    EXPECT_TRUE(parser->loadDir(QStringList { dir1.path(), dir2.path() }));

    auto entries = parser->getActionFiles(true);
    // 同名 conf 只解析一次，先遍历的目录优先
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().data().name(), "Test Action");
}

// ---- parseFile(QSettings &) ----

TEST_F(UT_DCustomActionParser, ParseFile_NoMenuEntryGroup_ReturnsFalse)
{
    writeConf(tempDir.filePath("noentry.conf"),
              "[Other Group]\n"
              "Version=1.0\n");
    QSettings setting(tempDir.filePath("noentry.conf"),
                      RegisterCustomFormat::instance().customFormat());
    EXPECT_FALSE(parser->parseFile(setting));
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_NoVersion_ReturnsFalse)
{
    writeConf(tempDir.filePath("noversion.conf"),
              "[Menu Entry]\n"
              "Actions=act1\n"
              "\n"
              "[Menu Action act1]\n"
              "Name=No Version\n"
              "Exec=/bin/echo\n");
    QSettings setting(tempDir.filePath("noversion.conf"),
                      RegisterCustomFormat::instance().customFormat());
    EXPECT_FALSE(parser->parseFile(setting));
}

TEST_F(UT_DCustomActionParser, ParseFile_EmptyActions_ReturnsFalse)
{
    writeConf(tempDir.filePath("noactions.conf"),
              "[Menu Entry]\n"
              "Version=1.0\n");
    QSettings setting(tempDir.filePath("noactions.conf"),
                      RegisterCustomFormat::instance().customFormat());
    EXPECT_FALSE(parser->parseFile(setting));
}

TEST_F(UT_DCustomActionParser, ParseFile_MoreThanFiftyTopActions_LimitedToFifty)
{
    QStringList actionNames;
    QString content = "[Menu Entry]\nVersion=1.0\nActions=";
    for (int i = 0; i < 60; ++i)
        actionNames << QString("a%1").arg(i);
    content += actionNames.join(":") + "\n";
    for (int i = 0; i < 60; ++i) {
        content += QString("\n[Menu Action a%1]\nName=Action %1\nExec=/bin/echo\n"
                           "X-DDE-FileManager-MenuTypes=SingleFile\n")
                           .arg(i);
    }
    writeConf(tempDir.filePath("too-many.conf"), content);

    QSettings setting(tempDir.filePath("too-many.conf"),
                      RegisterCustomFormat::instance().customFormat());
    EXPECT_TRUE(parser->parseFile(setting));
    // 一级菜单数量上限为 50
    EXPECT_EQ(parser->getActionFiles(true).size(), 50);
}

// ---- parseFile 递归重载（通过完整解析管线驱动）----

TEST_F(UT_DCustomActionParser, ParseFile_TopActionNameMissing_ReturnsFalse)
{
    // 缺少 [Menu Action act1] 分组，Name 为空
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"));
    EXPECT_TRUE(entry.version().isEmpty());
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_NoExecNoChildren_ReturnsFalse)
{
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=No Action\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n"));
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_HierarchyDeeperThanFour_Rejected)
{
    // 一级 act1 -> c1 -> c2 -> c3，超过三层子级
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Level1\n"
                                          "Actions=c1\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n"
                                          "\n"
                                          "[Menu Action c1]\n"
                                          "Name=Level2\n"
                                          "Actions=c2\n"
                                          "\n"
                                          "[Menu Action c2]\n"
                                          "Name=Level3\n"
                                          "Actions=c3\n"
                                          "\n"
                                          "[Menu Action c3]\n"
                                          "Name=Level4\n"
                                          "Exec=/bin/echo\n"));
    Q_UNUSED(entry)
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_TopActionWithoutCombo_Rejected)
{
    // 一级菜单未声明支持选中类型，视为无效
    parseSingleEntry(QString("[Menu Entry]\n"
                             "Version=1.0\n"
                             "Actions=act1\n"
                             "\n"
                             "[Menu Action act1]\n"
                             "Name=No Combo\n"
                             "Exec=/bin/echo\n"));
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_TopActionAllUnknownCombos_Rejected)
{
    // 一级菜单声明的选中类型全部未知，comboPosForTopAction 返回 false 后整项废弃
    parseSingleEntry(QString("[Menu Entry]\n"
                             "Version=1.0\n"
                             "Actions=act1\n"
                             "\n"
                             "[Menu Action act1]\n"
                             "Name=Unknown Combo\n"
                             "Exec=/bin/echo\n"
                             "X-DDE-FileManager-MenuTypes=BogusType\n"));
    EXPECT_TRUE(parser->getActionFiles(true).isEmpty());
}

TEST_F(UT_DCustomActionParser, ParseFile_SecondLevelLimit_HundredChildrenMax)
{
    // 二级菜单数量上限为 100
    QStringList childNames;
    QString content = "[Menu Entry]\nVersion=1.0\nActions=act1\n\n"
                      "[Menu Action act1]\nName=Top\nActions=";
    for (int i = 0; i < 101; ++i)
        childNames << QString("c%1").arg(i);
    content += childNames.join(":") + "\nX-DDE-FileManager-MenuTypes=SingleFile\n";
    for (int i = 0; i < 101; ++i)
        content += QString("\n[Menu Action c%1]\nName=Child %1\nExec=/bin/echo\n").arg(i);

    writeConf(tempDir.filePath("second-level.conf"), content);
    parser->menuPaths = QStringList { tempDir.path() };
    parser->refresh();

    auto entries = parser->getActionFiles(true);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.first().data().acitons().size(), 100);
}

TEST_F(UT_DCustomActionParser, ParseFile_ThirdLevelLimit_HundredChildrenMax)
{
    // 三级菜单数量上限为 100
    QStringList grandNames;
    QString content = "[Menu Entry]\nVersion=1.0\nActions=act1\n\n"
                      "[Menu Action act1]\nName=Top\nActions=mid\n"
                      "X-DDE-FileManager-MenuTypes=SingleFile\n\n"
                      "[Menu Action mid]\nName=Mid\nActions=";
    for (int i = 0; i < 101; ++i)
        grandNames << QString("g%1").arg(i);
    content += grandNames.join(":") + "\n";
    for (int i = 0; i < 101; ++i)
        content += QString("\n[Menu Action g%1]\nName=Grand %1\nExec=/bin/echo\n").arg(i);

    writeConf(tempDir.filePath("third-level.conf"), content);
    parser->menuPaths = QStringList { tempDir.path() };
    parser->refresh();

    auto entries = parser->getActionFiles(true);
    ASSERT_EQ(entries.size(), 1);
    const auto &top = entries.first().data();
    ASSERT_EQ(top.acitons().size(), 1);
    EXPECT_EQ(top.acitons().first().acitons().size(), 100);
}


TEST_F(UT_DCustomActionParser, ParseFile_TwoLevelMenu_ParsesChildrenAndSortsByPos)
{
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Parent Menu\n"
                                          "Actions=second:first\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile:SingleDir\n"
                                          "X-DDE-FileManager-PosNum=4\n"
                                          "\n"
                                          "[Menu Action second]\n"
                                          "Name=Second Child\n"
                                          "Exec=/bin/echo second\n"
                                          "PosNum=2\n"
                                          "\n"
                                          "[Menu Action first]\n"
                                          "Name=First Child\n"
                                          "Exec=/bin/echo first\n"
                                          "PosNum=1\n"));

    ASSERT_FALSE(entry.version().isEmpty());
    auto data = entry.data();
    EXPECT_TRUE(data.isMenu());
    EXPECT_EQ(data.name(), "Parent Menu");
    // 有子级时忽略自身的 Exec（此处本来就没写）
    EXPECT_EQ(data.position(), 4);
    EXPECT_EQ(data.position(kSingleFile), 4);
    EXPECT_EQ(data.position(kSingleDir), 4);
    EXPECT_TRUE(entry.fileCombo().testFlag(kSingleFile));
    EXPECT_TRUE(entry.fileCombo().testFlag(kSingleDir));

    // 子级按 PosNum 排序
    ASSERT_EQ(data.acitons().size(), 2);
    EXPECT_EQ(data.acitons().at(0).name(), "First Child");
    EXPECT_EQ(data.acitons().at(1).name(), "Second Child");
}

TEST_F(UT_DCustomActionParser, ParseFile_ChildWithoutPos_KeepsConfigOrder)
{
    // 某个子级无 PosNum 时取消排序，按 Actions 书写顺序排列
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Parent Menu\n"
                                          "Actions=first:second\n"
                                          "X-DDE-FileManager-MenuTypes=BlankSpace\n"
                                          "\n"
                                          "[Menu Action first]\n"
                                          "Name=Has Pos\n"
                                          "Exec=/bin/echo 1\n"
                                          "PosNum=2\n"
                                          "\n"
                                          "[Menu Action second]\n"
                                          "Name=No Pos\n"
                                          "Exec=/bin/echo 2\n"));

    auto children = entry.data().acitons();
    ASSERT_EQ(children.size(), 2);
    EXPECT_EQ(children.at(0).name(), "Has Pos");
    EXPECT_EQ(children.at(1).name(), "No Pos");
}

TEST_F(UT_DCustomActionParser, ParseFile_SeparatorAndParentPath_Parsed)
{
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=With Sep\n"
                                          "Exec=/bin/echo\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n"
                                          "Separator=Both\n"
                                          "X-DFM-ParentMenuPath=/custom/parent\n"));

    auto data = entry.data();
    EXPECT_EQ(data.separator(), kBoth);
    EXPECT_EQ(data.parentPath(), "/custom/parent");
}

TEST_F(UT_DCustomActionParser, ParseFile_LocalizedFullName_PreferredOverFallback)
{
    const QString systemLocale = QLocale::system().name().simplified();   // 如 "zh_CN"

    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Fallback Name\n"
                                          "Name[%1]=Localized Full\n"
                                          "Exec=/bin/echo\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n")
                                          .arg(systemLocale));

    EXPECT_EQ(entry.data().name(), "Localized Full");
}

TEST_F(UT_DCustomActionParser, ParseFile_LangOnlyName_FallbackChain)
{
    // 只提供语言简写 key（如 zh），命中 "语言_地区" 拆分后的回退分支；
    // 若系统 locale 本身不含 "_"，则完整 key 与语言 key 相同，依旧命中。
    const QString systemLocale = QLocale::system().name().simplified();
    const QString langOnly = systemLocale.split("_").first();

    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Fallback Name\n"
                                          "Name[%1]=Localized Lang\n"
                                          "Exec=/bin/echo\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n")
                                          .arg(langOnly));

    EXPECT_EQ(entry.data().name(), "Localized Lang");
}

TEST_F(UT_DCustomActionParser, ParseFile_ExecWithChildren_ChildrenWinOverCommand)
{
    auto entry = parseSingleEntry(QString("[Menu Entry]\n"
                                          "Version=1.0\n"
                                          "Actions=act1\n"
                                          "\n"
                                          "[Menu Action act1]\n"
                                          "Name=Both\n"
                                          "Exec=/bin/ignored\n"
                                          "Actions=child\n"
                                          "X-DDE-FileManager-MenuTypes=SingleFile\n"
                                          "\n"
                                          "[Menu Action child]\n"
                                          "Name=Child\n"
                                          "Exec=/bin/child\n"));

    auto data = entry.data();
    EXPECT_TRUE(data.isMenu());
    EXPECT_TRUE(data.command().isEmpty());
    ASSERT_EQ(data.acitons().size(), 1);
    EXPECT_EQ(data.acitons().first().command(), "/bin/child");
}

TEST_F(UT_DCustomActionParser, ParseFile_AllEntryOptions_WithAliasKeys)
{
    // 该配置含 NotShowIn=Desktop，需以非桌面视角取回
    writeConf(tempDir.filePath("alloptions.conf"),
              QString("[Menu Entry]\n"
                      "Version=3.2\n"
                      "Actions=act1\n"
                      "\n"
                      "[Menu Action act1]\n"
                      "Name=Full Options\n"
                      "Exec=/bin/echo %u\n"
                      "X-DFM-MenuTypes=MultiFiles\n"
                      "MimeType=text/plain:image/png\n"
                      "X-DFM-ExcludeMimeTypes=text/html\n"
                      "X-DFM-SupportSchemes=file:trash\n"
                      "X-DFM-NotShowIn=Desktop\n"
                      "X-DFM-SupportSuffix=*.7z.001\n"));
    parser->menuPaths = QStringList { tempDir.path() };
    parser->refresh();

    auto entries = parser->getActionFiles(false);
    ASSERT_EQ(entries.size(), 1);
    const DCustomActionEntry &entry = entries.first();

    EXPECT_TRUE(entry.fileCombo().testFlag(kMultiFiles));
    EXPECT_EQ(entry.mimeTypes(), QStringList({ "text/plain", "image/png" }));
    EXPECT_EQ(entry.excludeMimeTypes(), QStringList({ "text/html" }));
    EXPECT_EQ(entry.surpportSchemes(), QStringList({ "file", "trash" }));
    EXPECT_EQ(entry.notShowIn(), QStringList({ "Desktop" }));
    EXPECT_EQ(entry.supportStuffix(), QStringList({ "*.7z.001" }));
    EXPECT_EQ(entry.version(), "3.2");
    EXPECT_EQ(entry.data().commandArg(), kUrlPath);
}

// ---- getActionFiles ----

TEST_F(UT_DCustomActionParser, GetActionFiles_NotShowInFiltersEntries)
{
    auto makeConf = [](const QString &name, const QString &notShowIn) {
        return QString("[Menu Entry]\n"
                       "Version=1.0\n"
                       "Actions=act1\n"
                       "\n"
                       "[Menu Action act1]\n"
                       "Name=%1\n"
                       "Exec=/bin/echo\n"
                       "X-DDE-FileManager-MenuTypes=SingleFile\n"
                       "%2\n")
                .arg(name, notShowIn);
    };

    writeConf(tempDir.filePath("plain.conf"), makeConf("Plain", ""));
    writeConf(tempDir.filePath("no-desktop.conf"),
              makeConf("NoDesktop", "X-DDE-FileManager-NotShowIn=Desktop"));
    writeConf(tempDir.filePath("no-fm.conf"),
              makeConf("NoFileManager", "X-DFM-NotShowIn=Filemanager"));
    writeConf(tempDir.filePath("never.conf"), makeConf("Never", "X-DFM-NotShowIn=*"));

    parser->loadDir(QStringList { tempDir.path() });

    QStringList desktopNames, fmNames;
    for (const auto &entry : parser->getActionFiles(true))
        desktopNames << entry.data().name();
    for (const auto &entry : parser->getActionFiles(false))
        fmNames << entry.data().name();

    EXPECT_EQ(desktopNames.size(), 2);
    EXPECT_TRUE(desktopNames.contains("Plain"));
    EXPECT_TRUE(desktopNames.contains("NoFileManager"));

    EXPECT_EQ(fmNames.size(), 2);
    EXPECT_TRUE(fmNames.contains("Plain"));
    EXPECT_TRUE(fmNames.contains("NoDesktop"));
}

// ---- refresh / delayRefresh ----

TEST_F(UT_DCustomActionParser, Refresh_ReloadsFromMenuPaths)
{
    writeConf(tempDir.filePath("refresh.conf"), minimalValidConf());
    parser->menuPaths = QStringList { tempDir.path() };
    parser->refresh();
    EXPECT_EQ(parser->getActionFiles(true).size(), 1);

    // 新增配置后 refresh 重新加载
    QString another = QString("[Menu Entry]\n"
                              "Version=1.0\n"
                              "Actions=act1\n"
                              "\n"
                              "[Menu Action act1]\n"
                              "Name=Another\n"
                              "Exec=/bin/echo\n"
                              "X-DDE-FileManager-MenuTypes=BlankSpace\n");
    writeConf(tempDir.filePath("another.conf"), another);
    parser->refresh();
    EXPECT_EQ(parser->getActionFiles(true).size(), 2);
}

TEST_F(UT_DCustomActionParser, DelayRefresh_RestartsTimerAndReloadsOnTimeout)
{
    writeConf(tempDir.filePath("delay.conf"), minimalValidConf());
    parser->menuPaths = QStringList { tempDir.path() };

    // 第一次调用创建 300ms 定时器，第二次调用走 restart 分支
    parser->delayRefresh();
    parser->delayRefresh();

    QTest::qWait(400);

    EXPECT_EQ(parser->getActionFiles(true).size(), 1);
}

// ---- initWatcher ----

TEST_F(UT_DCustomActionParser, InitWatcher_XdgDataDirs_AppendedToMenuPaths)
{
    QTemporaryDir xdgRoot;
    ASSERT_TRUE(xdgRoot.isValid());
    const QString contextMenusDir = xdgRoot.filePath("applications/context-menus");
    ASSERT_TRUE(QDir(contextMenusDir).mkpath("."));

    const QByteArray oldXdg = qgetenv("XDG_DATA_DIRS");
    qputenv("XDG_DATA_DIRS", xdgRoot.path().toLocal8Bit());

    DCustomActionParser freshParser;
    EXPECT_TRUE(freshParser.menuPaths.contains(contextMenusDir));

    if (oldXdg.isEmpty())
        qunsetenv("XDG_DATA_DIRS");
    else
        qputenv("XDG_DATA_DIRS", oldXdg);
}

// ---- getValue ----

TEST_F(UT_DCustomActionParser, GetValue_ReturnsGroupKeyValue)
{
    writeConf(tempDir.filePath("getvalue.conf"),
              "[Menu Entry]\n"
              "Version=9.9\n");
    QSettings setting(tempDir.filePath("getvalue.conf"),
                      RegisterCustomFormat::instance().customFormat());

    EXPECT_EQ(parser->getValue(setting, "Menu Entry", "Version").toString(), "9.9");
    // 不存在的 key 返回无效 QVariant
    EXPECT_FALSE(parser->getValue(setting, "Menu Entry", "NotExists").isValid());
}

// ---- actionFileInfos ----

TEST_F(UT_DCustomActionParser, ActionFileInfos_NoVersion_ReturnsFalse)
{
    writeConf(tempDir.filePath("basicinfo.conf"),
              "[Menu Entry]\n"
              "Comment=no version here\n");
    QSettings setting(tempDir.filePath("basicinfo.conf"),
                      RegisterCustomFormat::instance().customFormat());

    FileBasicInfos infos;
    EXPECT_FALSE(parser->actionFileInfos(infos, setting));
    EXPECT_TRUE(infos.version.isEmpty());
}

TEST_F(UT_DCustomActionParser, ActionFileInfos_FillsBasicInfo)
{
    writeConf(tempDir.filePath("basicinfo2.conf"),
              "[Menu Entry]\n"
              "Version=5.5\n"
              "Comment=my comment\n"
              "Sign=my-sign\n");
    QSettings setting(tempDir.filePath("basicinfo2.conf"),
                      RegisterCustomFormat::instance().customFormat());

    FileBasicInfos infos;
    EXPECT_TRUE(parser->actionFileInfos(infos, setting));
    EXPECT_EQ(infos.version, "5.5");
    EXPECT_EQ(infos.comment, "my comment");
    EXPECT_EQ(infos.sign, "my-sign");
    EXPECT_EQ(infos.package, setting.fileName());
}

// ---- actionNameDynamicArg ----

TEST_F(UT_DCustomActionParser, ActionNameDynamicArg_AllArgTypes)
{
    struct
    {
        QString name;
        ActionArg expect;
    } cases[] = {
        { "%d folder", kDirName },
        { "%b basename", kBaseName },
        { "%a filename", kFileName },
        { "plain name", kNoneArg },
        { "%", kNoneArg },   // 长度为 1，cnt == 0
        { "x%", kNoneArg },   // % 在末位
        { "%y %b mixed", kBaseName },   // 跳过无效参数命中后续有效参数
        { "%y end", kNoneArg },   // 无效参数且再无 %，跳出循环
    };

    for (const auto &item : cases) {
        DCustomActionData act;
        act.actionName = item.name;
        parser->actionNameDynamicArg(act);
        EXPECT_EQ(act.nameArg(), item.expect) << "name: " << item.name.toStdString();
    }
}

// ---- execDynamicArg ----

TEST_F(UT_DCustomActionParser, ExecDynamicArg_AllArgTypes)
{
    struct
    {
        QString command;
        ActionArg expect;
    } cases[] = {
        { "/bin/x --dir %p", kDirPath },
        { "/bin/x %f", kFilePath },
        { "/bin/x %F", kFilePaths },
        { "/bin/x %u", kUrlPath },
        { "/bin/x %U", kUrlPaths },
        { "/bin/x %%", kNoneArg },   // %% 不是有效参数
        { "/bin/x noarg", kNoneArg },
        { "/bin/x %q only", kNoneArg },   // 无效参数且再无 %，跳出循环
    };

    for (const auto &item : cases) {
        DCustomActionData act;
        act.actionCommand = item.command;
        parser->execDynamicArg(act);
        EXPECT_EQ(act.commandArg(), item.expect) << "cmd: " << item.command.toStdString();
    }
}

// ---- comboPosForTopAction ----

TEST_F(UT_DCustomActionParser, ComboPosForTopAction_PerComboPosKeysParsed)
{
    writeConf(tempDir.filePath("combopos.conf"),
              "[Menu Entry]\n"
              "Version=1.0\n"
              "Actions=act1\n"
              "\n"
              "[Menu Action act1]\n"
              "X-DDE-FileManager-MenuTypes=SingleFile:SingleDir\n"
              "X-DDE-FileManager-PosNum-SingleFile=3\n"
              "PosNum-SingleDir=7\n");
    QSettings setting(tempDir.filePath("combopos.conf"),
                      RegisterCustomFormat::instance().customFormat());

    DCustomActionData act;
    act.actionPosition = 9;
    EXPECT_TRUE(parser->comboPosForTopAction(setting, "Menu Action act1", act));
    EXPECT_EQ(act.comboPos.value(kSingleFile), 3);
    EXPECT_EQ(act.comboPos.value(kSingleDir), 7);
}

TEST_F(UT_DCustomActionParser, ComboPosForTopAction_NoPosKey_FallsBackToActionPosition)
{
    writeConf(tempDir.filePath("combopos2.conf"),
              "[Menu Entry]\n"
              "Version=1.0\n"
              "Actions=act1\n"
              "\n"
              "[Menu Action act1]\n"
              "X-DDE-FileManager-MenuTypes=SingleFile\n");
    QSettings setting(tempDir.filePath("combopos2.conf"),
                      RegisterCustomFormat::instance().customFormat());

    DCustomActionData act;
    act.actionPosition = 9;
    EXPECT_TRUE(parser->comboPosForTopAction(setting, "Menu Action act1", act));
    EXPECT_EQ(act.comboPos.value(kSingleFile), 9);
}

TEST_F(UT_DCustomActionParser, ComboPosForTopAction_AllUnknownCombos_ReturnsFalse)
{
    writeConf(tempDir.filePath("combopos3.conf"),
              "[Menu Entry]\n"
              "Version=1.0\n"
              "Actions=act1\n"
              "\n"
              "[Menu Action act1]\n"
              "X-DDE-FileManager-MenuTypes=BogusType\n");
    QSettings setting(tempDir.filePath("combopos3.conf"),
                      RegisterCustomFormat::instance().customFormat());

    DCustomActionData act;
    EXPECT_FALSE(parser->comboPosForTopAction(setting, "Menu Action act1", act));
    EXPECT_TRUE(act.comboPos.isEmpty());
}

// ---- isActionShouldShow ----

TEST_F(UT_DCustomActionParser, IsActionShouldShow_AllBranches)
{
    // 未配置默认都显示
    EXPECT_TRUE(DCustomActionParser::isActionShouldShow(QStringList(), true));
    EXPECT_TRUE(DCustomActionParser::isActionShouldShow(QStringList(), false));

    // "*" 都不显示
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "*" }, true));
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "*" }, false));

    // 桌面隐藏
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "Desktop" }, true));
    EXPECT_TRUE(DCustomActionParser::isActionShouldShow(QStringList { "Desktop" }, false));

    // 文管隐藏
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "Filemanager" }, false));
    EXPECT_TRUE(DCustomActionParser::isActionShouldShow(QStringList { "Filemanager" }, true));

    // 大小写不敏感
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "desktop" }, true));
    EXPECT_FALSE(DCustomActionParser::isActionShouldShow(QStringList { "filemanager" }, false));
}

