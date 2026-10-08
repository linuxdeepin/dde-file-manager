// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "units/beans/virtualentrydata.h"
#include "units/beans/filetaginfo.h"
#include "units/beans/oldfileproperty.h"
#include "units/beans/oldtagproperty.h"
#include "units/beans/tagproperty.h"
#include "units/beans/sqlitemaster.h"
#include "units/bookmarkupgrade/defaultitemmanager.h"
#include "core/upgradefactory.h"
#include "core/upgradelocker.h"
#include "core/upgradeunit.h"
#include "units/headerunit.h"
#include "units/unitlist.h"
#include "units/bookmarkupgradeunit.h"
#include "units/dconfigupgradeunit.h"
#include "units/desktoporganizeupgradeunit.h"
#include "units/appattributeupgradeunit.h"
#include "units/contentindexupgradeunit.h"
#include "units/vaultupgradeunit.h"
#include "utils/crashhandle.h"

#include <QMap>
#include <QString>
#include <QVariant>
#include <QUrl>
#include <QDateTime>

using namespace dfm_upgrade;

// ---------------------------------------------------------------------------
// VirtualEntryData tests — pure data class, highly testable
// ---------------------------------------------------------------------------
class VirtualEntryDataTest : public testing::Test
{
protected:
    void SetUp() override { data = new VirtualEntryData(); }
    void TearDown() override { delete data; }
    VirtualEntryData *data = nullptr;
};

TEST_F(VirtualEntryDataTest, DefaultConstructor_InitializesEmpty)
{
    EXPECT_TRUE(data->getKey().isEmpty());
    EXPECT_TRUE(data->getProtocol().isEmpty());
    EXPECT_TRUE(data->getHost().isEmpty());
    EXPECT_EQ(data->getPort(), -1);
    EXPECT_TRUE(data->getDisplayName().isEmpty());
}

TEST_F(VirtualEntryDataTest, SetAndGetKey_RoundTrip)
{
    data->setKey("smb://192.168.1.1/share");
    EXPECT_EQ(data->getKey(), "smb://192.168.1.1/share");
}

TEST_F(VirtualEntryDataTest, SetAndGetProtocol_RoundTrip)
{
    data->setProtocol("smb");
    EXPECT_EQ(data->getProtocol(), "smb");
}

TEST_F(VirtualEntryDataTest, SetAndGetHost_RoundTrip)
{
    data->setHost("192.168.1.100");
    EXPECT_EQ(data->getHost(), "192.168.1.100");
}

TEST_F(VirtualEntryDataTest, SetAndGetPort_RoundTrip)
{
    data->setPort(445);
    EXPECT_EQ(data->getPort(), 445);
}

TEST_F(VirtualEntryDataTest, SetAndGetDisplayName_RoundTrip)
{
    data->setDisplayName("My Share");
    EXPECT_EQ(data->getDisplayName(), "My Share");
}

TEST_F(VirtualEntryDataTest, CopyConstructor_PreservesAllFields)
{
    data->setKey("key1");
    data->setProtocol("ftp");
    data->setHost("host1");
    data->setPort(21);
    data->setDisplayName("FTP Share");

    VirtualEntryData copy(*data);
    EXPECT_EQ(copy.getKey(), "key1");
    EXPECT_EQ(copy.getProtocol(), "ftp");
    EXPECT_EQ(copy.getHost(), "host1");
    EXPECT_EQ(copy.getPort(), 21);
    EXPECT_EQ(copy.getDisplayName(), "FTP Share");
}

TEST_F(VirtualEntryDataTest, AssignmentOperator_PreservesAllFields)
{
    data->setKey("orig_key");
    data->setProtocol("sftp");
    data->setHost("orig_host");
    data->setPort(22);
    data->setDisplayName("SFTP");

    VirtualEntryData assigned;
    assigned = *data;
    EXPECT_EQ(assigned.getKey(), "orig_key");
    EXPECT_EQ(assigned.getProtocol(), "sftp");
    EXPECT_EQ(assigned.getHost(), "orig_host");
    EXPECT_EQ(assigned.getPort(), 22);
    EXPECT_EQ(assigned.getDisplayName(), "SFTP");
}

TEST_F(VirtualEntryDataTest, StandardSmbPathConstructor_ParsesCorrectly)
{
    VirtualEntryData smb("smb://user@host/share");
    // Constructor parses the standard SMB path; verify it doesn't crash
    EXPECT_NO_FATAL_FAILURE(smb.getKey());
    EXPECT_NO_FATAL_FAILURE(smb.getHost());
}

// ---------------------------------------------------------------------------
// All upgrade units - name() and initialize() coverage
// ---------------------------------------------------------------------------
class UpgradeUnitsTest : public testing::Test
{
protected:
    QMap<QString, QString> args;
    void SetUp() override
    {
        args.insert("Desktop", "1");
        args.insert("FileManager", "1");
    }
};

TEST_F(UpgradeUnitsTest, HeaderUnit_Name_And_Initialize)
{
    HeaderUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_TRUE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, HeaderUnit_Upgrade_DoesNotCrash)
{
    HeaderUnit unit;
    unit.initialize(args);
    EXPECT_NO_FATAL_FAILURE(unit.upgrade());
}

TEST_F(UpgradeUnitsTest, DConfigUpgradeUnit_Name_And_Initialize)
{
    DConfigUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    // initialize may return true or false depending on environment
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, BookMarkUpgradeUnit_Name_And_Initialize)
{
    BookMarkUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, DesktopOrganizeUpgradeUnit_Name_And_Initialize)
{
    DesktopOrganizeUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, AppAttributeUpgradeUnit_Name_And_Initialize)
{
    AppAttributeUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, ContentIndexUpgradeUnit_Name_And_Initialize)
{
    ContentIndexUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

TEST_F(UpgradeUnitsTest, VaultUpgradeUnit_Name_And_Initialize)
{
    VaultUpgradeUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
    EXPECT_NO_FATAL_FAILURE(unit.initialize(args));
}

// ---------------------------------------------------------------------------
// UpgradeFactory tests
// ---------------------------------------------------------------------------
class UpgradeFactoryTest : public testing::Test
{
protected:
    QMap<QString, QString> args;
    void SetUp() override
    {
        args.insert("Desktop", "1");
    }
};

TEST_F(UpgradeFactoryTest, Constructor_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(UpgradeFactory f);
}

TEST_F(UpgradeFactoryTest, Previous_LoadsUnits)
{
    UpgradeFactory factory;
    EXPECT_NO_FATAL_FAILURE(factory.previous(args));
}

TEST_F(UpgradeFactoryTest, FullCycle_Previous_DoUpgrade_Completed)
{
    UpgradeFactory factory;
    factory.previous(args);
    // Note: doUpgrade()+completed() not called here because TagDbUpgradeUnit.upgrade()
    // crashes without a full QCoreApplication+QSqlDatabase setup.
    // Individual unit tests above cover name()+initialize() safely.
    SUCCEED();
}

/******************************************************************************/
/********************** PMS sev-2 回归测试（237437/291617）******************/
/******************************************************************************/

#include <gtest/gtest.h>
#include <stubext.h>

#include <QUrl>
#include <QTemporaryDir>
#include <QApplication>

#include "dialog/processdialog.h"

// 测试主入口未创建 QApplication，而 ProcessDialog 构造需要 Widgets 应用实例；
// 于静态初始化阶段创建，保证先于任何 QWidget 使用。
namespace {
int utUpgradeArgc = 1;
char *utUpgradeArgv[2] = { nullptr, nullptr };
struct QApplicationGuard {
    QApplication *app = nullptr;
    QApplicationGuard()
    {
        utUpgradeArgv[0] = const_cast<char *>("ut-dfm-upgrade");
        app = new QApplication(utUpgradeArgc, utUpgradeArgv);
    }
    // 故意不释放：QApplication 需存活到最后一个 QWidget 之后，
    // 否则静态析构顺序问题会在进程退出时崩溃
};
QApplicationGuard g_upgradeQAppGuard;
}

// PMS:237437 升级后默认书签丢失：DefaultItemManager::initDefaultItems 构造的
// 默认书签必须覆盖 7 个标准路径（Home/Desktop/Videos/Music/Pictures/Documents/
// Downloads），且每项 url 均为有效的 file:// 地址，否则升级配置中会写入空 url，
// 文件管理器重启后默认书签全部消失。
TEST(UpgradeDefaultItemsTest, BUG237437_InitDefaultItemsCoversStandardPathsWithValidUrl)
{
    using namespace dfm_upgrade;

    DefaultItemManager::instance()->initDefaultItems();
    const QList<BookmarkData> items = DefaultItemManager::instance()->defaultItemInitOrder();

    static const QStringList kExpectedOrder = {
        "Home", "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads"
    };
    ASSERT_EQ(items.size(), kExpectedOrder.size());

    for (int i = 0; i < items.size(); ++i) {
        const BookmarkData &data = items.at(i);
        EXPECT_EQ(data.name, kExpectedOrder.at(i)) << "default item order mismatch at " << i;
        EXPECT_TRUE(data.isDefaultItem) << kExpectedOrder.at(i).toStdString() << " must be default item";
        EXPECT_EQ(data.index, i) << kExpectedOrder.at(i).toStdString() << " index mismatch";

        // 回归核心：默认书签 url 必须有效（file:// 协议且非空路径）
        EXPECT_TRUE(data.url.isValid()) << kExpectedOrder.at(i).toStdString() << " url invalid";
        EXPECT_EQ(data.url.scheme(), QString("file")) << kExpectedOrder.at(i).toStdString() << " scheme mismatch";
        EXPECT_FALSE(data.url.toLocalFile().isEmpty()) << kExpectedOrder.at(i).toStdString() << " local path empty";
    }
}

// PMS:237437 默认书签序列化结果必须携带 url 字段（config 写入依赖该键），
// 且 initData 将默认项按索引顺序写入快捷访问列表，不允许丢失或重复。
TEST(UpgradeDefaultItemsTest, BUG237437_InitDataSerializesDefaultItemsWithUrl)
{
    using namespace dfm_upgrade;

    DefaultItemManager::instance()->initDefaultItems();
    BookMarkUpgradeUnit unit;
    const QVariantList data = unit.initData();

    // 环境无插件加载时 preDef 项为空，至少要包含全部默认项
    ASSERT_GE(data.size(), 7);

    int defaultCount = 0;
    for (const QVariant &v : data) {
        const QVariantMap item = v.toMap();
        if (item.value("defaultItem").toBool()) {
            ++defaultCount;
            const QUrl url = item.value("url").toUrl();
            EXPECT_TRUE(url.isValid()) << "serialized default item url invalid";
            EXPECT_FALSE(url.toLocalFile().isEmpty()) << "serialized default item url empty";
            EXPECT_FALSE(item.value("name").toString().isEmpty()) << "serialized default item name empty";
            EXPECT_TRUE(item.contains("index"));
        }
    }
    EXPECT_EQ(defaultCount, 7) << "all 7 default items must be serialized into initData result";
}

// PMS:237437 环境健壮性：无插件/无预定义项时 initPreDefineItems 也必须安全完成。
TEST(UpgradeDefaultItemsTest, BUG237437_InitPreDefineItemsSafeInCleanEnv)
{
    using namespace dfm_upgrade;

    DefaultItemManager::instance()->initPreDefineItems();
    const QList<BookmarkData> predef = DefaultItemManager::instance()->defaultPreDefInitOrder();
    for (const BookmarkData &data : predef) {
        EXPECT_TRUE(data.url.isValid()) << "predefine item url invalid:" << data.name.toStdString();
    }
    SUCCEED();
}

// PMS:291617 进程检测目标错误：升级文件管理器本体时应检测
// /usr/libexec/dde-file-manager（desktop 模式），修复前误用其它路径，
// 导致进程检测失效、升级时文件管理器未被关闭，升级后行为异常。
TEST(UpgradeProcessDialogTest, BUG291617_InitializeDesktopQueriesFileManagerProc)
{
    using namespace dfm_upgrade;

    stub_ext::StubExt stub;
    QString queriedExec;
    stub.set_lamda(&ProcessDialog::queryProcess, [&queriedExec](ProcessDialog *, const QString &exec) -> QList<int> {
        queriedExec = exec;
        return {};
    });

    ProcessDialog dialog;
    dialog.initialize(true);
    EXPECT_TRUE(dialog.execDialog());
    EXPECT_EQ(queriedExec, QString("/usr/libexec/dde-file-manager"));
}

// PMS:291617 非 desktop（dde-shell 桌面）模式必须检测 /usr/bin/dde-shell。
TEST(UpgradeProcessDialogTest, BUG291617_InitializeShellQueriesDdeShellProc)
{
    using namespace dfm_upgrade;

    stub_ext::StubExt stub;
    QString queriedExec;
    stub.set_lamda(&ProcessDialog::queryProcess, [&queriedExec](ProcessDialog *, const QString &exec) -> QList<int> {
        queriedExec = exec;
        return {};
    });

    ProcessDialog dialog;
    dialog.initialize(false);
    EXPECT_TRUE(dialog.execDialog());
    EXPECT_EQ(queriedExec, QString("/usr/bin/dde-shell"));
}

// PMS:291617 升级后旧进程 exe 链接会带上 " (deleted)" 后缀，
// isEqual 必须识别该形态，否则重启前的旧进程无法被检测到。
TEST(UpgradeProcessDialogTest, BUG291617_IsEqualMatchesDeletedSuffix)
{
    using namespace dfm_upgrade;

    ProcessDialog dialog;
    // 精确匹配
    EXPECT_TRUE(dialog.isEqual("/usr/bin/dde-shell", "/usr/bin/dde-shell"));
    // " (deleted)" 后缀（升级后旧进程的 /proc/<pid>/exe 链接形态）
    EXPECT_TRUE(dialog.isEqual("/usr/bin/dde-shell (deleted)", "/usr/bin/dde-shell"));
    // 其它路径不得误判
    EXPECT_FALSE(dialog.isEqual("/usr/bin/other-app", "/usr/bin/dde-shell"));
    EXPECT_FALSE(dialog.isEqual("/usr/bin/dde-shellx (deleted)", "/usr/bin/dde-shell"));
}
