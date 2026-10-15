// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "core/upgradefactory.h"
#include "core/upgradeunit.h"
#include "core/upgradelocker.h"
#include "dialog/processdialog.h"
#include "units/bookmarkupgradeunit.h"
#include "units/bookmarkupgrade/defaultitemmanager.h"
#include "units/appattributeupgradeunit.h"
#include "units/contentindexupgradeunit.h"
#include "units/desktoporganizeupgradeunit.h"
#include "units/vaultupgradeunit.h"
#include "units/dconfigupgradeunit.h"
#include "units/unitlist.h"
#include "utils/crashhandle.h"
#include "upgradeinterface.h"
#include "builtininterface.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QApplication>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSignalSpy>

#include <unistd.h>
#include <csignal>

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------
namespace {
QString sandboxConfigPath(const QString &relative)
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + relative;
}

bool writeJsonFile(const QString &path, const QJsonObject &root)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QJsonDocument(root).toJson());
    f.close();
    return true;
}

QJsonObject readJsonFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}
}   // namespace

// ---------------------------------------------------------------------------
// UpgradeUnit base class + UpgradeFactory
// ---------------------------------------------------------------------------
class FakeUnit final : public UpgradeUnit
{
public:
    explicit FakeUnit(bool initOk, bool upgradeOk)
        : initResult(initOk), upgradeResult(upgradeOk) {}
    QString name() override { return "FakeUnit"; }
    bool initialize(const QMap<QString, QString> &) override { initCalls++; return initResult; }
    bool upgrade() override { upgradeCalls++; return upgradeResult; }
    int initCalls = 0;
    int upgradeCalls = 0;
    bool initResult = true;
    bool upgradeResult = true;
};

class UpgradeFactoryCovTest : public testing::Test
{
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(UpgradeFactoryCovTest, BaseUnit_CompletedAndDestructor_Covered)
{
    // Arrange: PlainUnit does not override completed() -> base impl runs
    struct PlainUnit : UpgradeUnit {
        QString name() override { return "PlainUnit"; }
        bool initialize(const QMap<QString, QString> &) override { return true; }
        bool upgrade() override { return true; }
    };

    // Act
    UpgradeUnit *unit = new PlainUnit();
    unit->completed();
    delete unit;   // virtual ~UpgradeUnit()

    // Assert
    SUCCEED();
}

TEST_F(UpgradeFactoryCovTest, DoUpgrade_And_Completed_InvokeEveryUnit)
{
    // Arrange
    UpgradeFactory factory;
    auto okUnit = QSharedPointer<UpgradeUnit>(new FakeUnit(true, true));
    auto failUnit = QSharedPointer<UpgradeUnit>(new FakeUnit(true, false));
    factory.units << okUnit << failUnit;
    auto *ok = static_cast<FakeUnit *>(okUnit.data());
    auto *bad = static_cast<FakeUnit *>(failUnit.data());

    // Act
    factory.doUpgrade();
    factory.completed();

    // Assert: iteration continues past the failing unit
    EXPECT_EQ(ok->upgradeCalls, 1);
    EXPECT_EQ(bad->upgradeCalls, 1);
    EXPECT_TRUE(factory.units.contains(failUnit));
}

TEST_F(UpgradeFactoryCovTest, Previous_ErasesUnitWhenInitializeFails)
{
    // Arrange: run the real unit list inside the sandbox HOME; most units
    // initialize() succeed, units that need a legacy config fail & are erased
    UpgradeFactory factory;
    QMap<QString, QString> args { { "Desktop", "1" } };
    // dconfig writes are stubbed to keep the sandbox free of dconf side effects
    stub.set_lamda(ADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                   });

    // Act
    factory.previous(args);
    const int remaining = factory.units.size();
    factory.doUpgrade();
    factory.completed();

    // Assert: all remaining units survived initialize()
    EXPECT_GT(remaining, 0);
    EXPECT_EQ(factory.units.size(), remaining);
}

// ---------------------------------------------------------------------------
// VaultUpgradeUnit
// ---------------------------------------------------------------------------
class VaultUpgradeCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // ctor derives all paths from $HOME, which is the sandbox home;
        // wipe both vault locations so scenarios do not leak into each other
        QDir(QDir::homePath() + "/.config/Vault").removeRecursively();
        QDir(QDir::homePath() + "/.local/share/applications/" + kVaultEncrypyDirName).removeRecursively();
        QDir(QDir::homePath() + "/.local/share/applications/" + kVaultDecryptDirName).removeRecursively();
        unit = new VaultUpgradeUnit();
    }
    void TearDown() override
    {
        delete unit;
        stub.clear();
    }

    // build a legacy vault under $HOME/.local/share/applications
    void createLegacyVault()
    {
        const QString oldBase = QDir::homePath() + "/.local/share/applications";
        QDir().mkpath(oldBase + "/" + kVaultEncrypyDirName);
        QDir().mkpath(oldBase + "/" + kVaultDecryptDirName);
        QFile cryfs(oldBase + "/" + kVaultEncrypyDirName + "/" + kCryfsConfigFileName);
        ASSERT_TRUE(cryfs.open(QIODevice::WriteOnly));
        cryfs.close();
        QFile decrypt(oldBase + "/" + kVaultDecryptDirName + "/keep.txt");
        ASSERT_TRUE(decrypt.open(QIODevice::WriteOnly));
        decrypt.write("vault content");
        decrypt.close();
    }

    VaultUpgradeUnit *unit { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(VaultUpgradeCovTest, Name_And_Initialize_DependsOnLegacyCryfsConfig)
{
    // Arrange: no legacy vault
    QMap<QString, QString> args;

    // Act / Assert: nothing to upgrade
    EXPECT_EQ(unit->name(), QString("VaultUpgradeUnit"));
    EXPECT_FALSE(unit->initialize(args));

    // Arrange: legacy cryfs config present, new one absent
    createLegacyVault();

    // Act / Assert
    EXPECT_TRUE(unit->initialize(args));

    // Arrange: new config already exists -> done before
    const QString newConfig = QDir::homePath() + "/.config/Vault/"
            + QString(kVaultEncrypyDirName) + "/" + kCryfsConfigFileName;
    QDir().mkpath(QFileInfo(newConfig).absolutePath());
    QFile created(newConfig);
    created.open(QIODevice::WriteOnly);
    created.close();

    // Act / Assert
    EXPECT_FALSE(unit->initialize(args));
}

TEST_F(VaultUpgradeCovTest, Upgrade_NewVaultExists_Aborts)
{
    // Arrange
    createLegacyVault();
    QDir().mkpath(QDir::homePath() + "/.config/Vault");

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(VaultUpgradeCovTest, Upgrade_LegacyVault_MovedToNewLocation)
{
    // Arrange
    createLegacyVault();

    // Act
    const bool ok = unit->upgrade();

    // Assert: decrypt dir moved into the new vault base
    EXPECT_TRUE(ok);
    const QString moved = QDir::homePath() + "/.config/Vault/" + kVaultDecryptDirName + "/keep.txt";
    EXPECT_TRUE(QFile::exists(moved));
    EXPECT_TRUE(QFile::exists(QDir::homePath() + "/.config/Vault/" + kVaultEncrypyDirName + "/"
                              + kCryfsConfigFileName));

    // Act: completed() is a pure notification hook
    EXPECT_NO_FATAL_FAILURE(unit->completed());
}

TEST_F(VaultUpgradeCovTest, MoveVault_WithoutLegacyData_OnlyCreatesBaseDir)
{
    // Arrange: nothing exists at all

    // Act
    EXPECT_NO_FATAL_FAILURE(unit->moveVault());

    // Assert
    EXPECT_TRUE(QDir(QDir::homePath() + "/.config/Vault").exists());
}

TEST_F(VaultUpgradeCovTest, IsLockState_MissingPathOrPlainDir_NotLocked)
{
    // Arrange
    QDir().mkpath(QDir::homePath() + "/plain-dir");

    // Act / Assert
    EXPECT_FALSE(unit->isLockState(QDir::homePath() + "/definitely-not-exist"));
    EXPECT_FALSE(unit->isLockState(QDir::homePath() + "/plain-dir"));   // not fuse.cryfs
}

TEST_F(VaultUpgradeCovTest, LockVault_NoFusermountBinary_Fails)
{
    // Arrange: force findExecutable to fail for determinism
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString();
                   });

    // Act
    const bool locked = unit->lockVault(QDir::homePath() + "/some-mount");

    // Assert
    EXPECT_FALSE(locked);
}

// ---------------------------------------------------------------------------
// AppAttributeUpgradeUnit
// ---------------------------------------------------------------------------
class AppAttributeCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // UpgradeUnit ctor: $CONFIG/deepin/dde-file-manager/dde-file-manager.json
        unit = new AppAttributeUpgradeUnit();
        // start every scenario from a clean slate
        QFile::remove(unit->backupDirPath + "/ApplicationAttribute_v1.0.backup");
        QFile::remove(unit->configurationPath);
    }
    void TearDown() override { delete unit; }

    void writeConfig(int iconSizeLevel)
    {
        QDir().mkpath(QFileInfo(unit->configurationPath).absolutePath());
        QJsonObject appAttr;
        appAttr.insert("IconSizeLevel", iconSizeLevel);
        QJsonObject root;
        root.insert("ApplicationAttribute", appAttr);
        ASSERT_TRUE(writeJsonFile(unit->configurationPath, root));
    }

    AppAttributeUpgradeUnit *unit { nullptr };
};

TEST_F(AppAttributeCovTest, TransIconSizeLevel_AllLevels)
{
    // Arrange / Act / Assert
    EXPECT_EQ(unit->transIconSizeLevel(0), 3);
    EXPECT_EQ(unit->transIconSizeLevel(1), 5);
    EXPECT_EQ(unit->transIconSizeLevel(2), 9);
    EXPECT_EQ(unit->transIconSizeLevel(3), 13);
    EXPECT_EQ(unit->transIconSizeLevel(4), 17);
    EXPECT_EQ(unit->transIconSizeLevel(9), 9);   // out of range -> unchanged
}

TEST_F(AppAttributeCovTest, Initialize_ConfigVariants_AcceptedOrRejected)
{
    // Arrange
    QMap<QString, QString> args;

    // no config file -> false
    QFile::remove(unit->configurationPath);
    EXPECT_FALSE(unit->initialize(args));

    // config without ApplicationAttribute -> false (backup made, remove it)
    ASSERT_TRUE(writeJsonFile(unit->configurationPath, QJsonObject()));
    EXPECT_FALSE(unit->initialize(args));
    QFile::remove(unit->backupDirPath + "/ApplicationAttribute_v1.0.backup");

    // IconSizeLevel out of the legacy range -> already migrated
    writeConfig(9);
    EXPECT_FALSE(unit->initialize(args));
    QFile::remove(unit->backupDirPath + "/ApplicationAttribute_v1.0.backup");

    // valid legacy value
    writeConfig(2);
    EXPECT_TRUE(unit->initialize(args));
    EXPECT_EQ(unit->oldIconSizeLevel, 2);

    // a second initialize fails: the backup file already exists
    EXPECT_FALSE(unit->initialize(args));
}

TEST_F(AppAttributeCovTest, Upgrade_WritesTranslatedIconSize)
{
    // Arrange
    writeConfig(1);
    ASSERT_TRUE(unit->initialize({}));

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    const QJsonObject root = readJsonFile(unit->configurationPath);
    EXPECT_EQ(root.value("ApplicationAttribute").toObject().value("IconSizeLevel").toInt(), 5);

    EXPECT_NO_FATAL_FAILURE(unit->completed());

    // backup file was created next to the config
    const QString backup = unit->backupDirPath + "/ApplicationAttribute_v1.0.backup";
    EXPECT_TRUE(QFile::exists(backup));
}

TEST_F(AppAttributeCovTest, Upgrade_MissingSection_Fails)
{
    // Arrange: initialized config object without the section
    writeConfig(1);
    ASSERT_TRUE(unit->initialize({}));
    unit->configObject = QJsonObject();

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(AppAttributeCovTest, WriteConfigFile_UnwritablePath_Fails)
{
    // Arrange
    writeConfig(1);
    ASSERT_TRUE(unit->initialize({}));
    unit->configurationPath = QDir::homePath() + "/missing-dir-xyz/sub/config.json";

    // Act
    const bool ok = unit->writeConfigFile();

    // Assert
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// ContentIndexUpgradeUnit
// ---------------------------------------------------------------------------
class ContentIndexCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        unit = new ContentIndexUpgradeUnit();
        // start every scenario with a clean target directory
        QDir(unit->newIndexDirPath).removeRecursively();
        QFile::remove(unit->newStatusFilePath);
        QDir().mkpath(unit->oldIndexDirPath);
    }
    void TearDown() override { delete unit; }

    void writeOldStatus()
    {
        QFile f(unit->oldStatusFilePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("{}");
        f.close();
    }

    ContentIndexUpgradeUnit *unit { nullptr };
};

TEST_F(ContentIndexCovTest, Name_And_Initialize_StatusFileGuards)
{
    // Arrange
    QMap<QString, QString> args;
    EXPECT_EQ(unit->name(), QString("ContentIndexUpgradeUnit"));

    // no status file -> skip
    EXPECT_FALSE(unit->initialize(args));

    // status file present -> upgrade wanted
    writeOldStatus();
    EXPECT_TRUE(unit->initialize(args));

    // new status file already exists -> done before
    QDir().mkpath(QFileInfo(unit->newStatusFilePath).absolutePath());
    QFile done(unit->newStatusFilePath);
    done.open(QIODevice::WriteOnly);
    done.close();
    EXPECT_FALSE(unit->initialize(args));
}

TEST_F(ContentIndexCovTest, CanMoveOldIndexDirectory_AllBranches)
{
    // Arrange / Act / Assert: target missing -> movable
    EXPECT_TRUE(unit->canMoveOldIndexDirectory());

    // target is a file -> refuse
    QDir().mkpath(QFileInfo(unit->newIndexDirPath).absolutePath());
    QFile asFile(unit->newIndexDirPath);
    asFile.open(QIODevice::WriteOnly);
    asFile.close();
    EXPECT_FALSE(unit->canMoveOldIndexDirectory());
    asFile.remove();

    // target dir non-empty -> refuse
    QDir().mkpath(unit->newIndexDirPath);
    QFile inner(unit->newIndexDirPath + "/occupied");
    inner.open(QIODevice::WriteOnly);
    inner.close();
    EXPECT_FALSE(unit->canMoveOldIndexDirectory());
    inner.remove();

    // target dir empty -> removed then movable
    EXPECT_TRUE(unit->canMoveOldIndexDirectory());
    EXPECT_FALSE(QDir(unit->newIndexDirPath).exists());
}

TEST_F(ContentIndexCovTest, Upgrade_MovesIndexDirectory)
{
    // Arrange
    writeOldStatus();
    QFile indexData(unit->oldIndexDirPath + "/data.bin");
    ASSERT_TRUE(indexData.open(QIODevice::WriteOnly));
    indexData.write("index");
    indexData.close();

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile::exists(unit->newIndexDirPath + "/data.bin"));
    EXPECT_FALSE(QDir(unit->oldIndexDirPath).exists());
    EXPECT_NO_FATAL_FAILURE(unit->completed());
}

TEST_F(ContentIndexCovTest, Upgrade_BlockedTarget_Fails)
{
    // Arrange: non-empty target refuses the move
    writeOldStatus();
    QDir().mkpath(unit->newIndexDirPath);
    QFile inner(unit->newIndexDirPath + "/occupied");
    inner.open(QIODevice::WriteOnly);
    inner.close();

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// DesktopOrganizeUpgradeUnit
// ---------------------------------------------------------------------------
class DesktopOrganizeCovTest : public testing::Test
{
protected:
    void SetUp() override { unit = new DesktopOrganizeUpgradeUnit(); }
    void TearDown() override { delete unit; }
    DesktopOrganizeUpgradeUnit *unit { nullptr };
};

TEST_F(DesktopOrganizeCovTest, Upgrade_RemovesOrganizerConfig)
{
    // Arrange
    QDir().mkpath(QFileInfo(unit->cfgPath).absolutePath());
    QFile f(unit->cfgPath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("[General]\n");
    f.close();

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_FALSE(QFile::exists(unit->cfgPath));
}

TEST_F(DesktopOrganizeCovTest, Upgrade_MissingFile_Fails)
{
    // Arrange
    QFile::remove(unit->cfgPath);

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// BookMarkUpgradeUnit
// ---------------------------------------------------------------------------
class BookMarkCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        unit = new BookMarkUpgradeUnit();
        QDir().mkpath(QFileInfo(unit->configurationPath).absolutePath());
    }
    void TearDown() override { delete unit; }
    BookMarkUpgradeUnit *unit { nullptr };
};

TEST_F(BookMarkCovTest, Initialize_AlreadyUpgradedConfig_ReturnsFalse)
{
    // Arrange
    QJsonObject quick;
    quick.insert("Items", QJsonArray());
    QJsonObject root;
    root.insert("QuickAccess", quick);
    ASSERT_TRUE(writeJsonFile(unit->configurationPath, root));

    // Act
    const bool inited = unit->initialize({});

    // Assert
    EXPECT_FALSE(inited);
}

TEST_F(BookMarkCovTest, Upgrade_LegacyBookmarks_MigratedToQuickAccess)
{
    // Arrange: legacy config with one bookmark + its sort order
    QVariantHash item;
    item.insert("name", QString("projects"));
    item.insert("url", QUrl("file:///home/user/projects"));
    QJsonArray items;
    items.append(QJsonObject::fromVariantHash(item));
    QJsonObject bookMark;
    bookMark.insert("Items", items);
    QJsonObject bookmarkOrder;
    bookmarkOrder.insert("bookmark", QJsonArray { QString("bookmark:file:///home/user/projects") });
    QJsonObject root;
    root.insert("BookMark", bookMark);
    root.insert("SideBar", QJsonObject({ { "ItemOrder", bookmarkOrder } }));
    ASSERT_TRUE(writeJsonFile(unit->configurationPath, root));
    ASSERT_TRUE(unit->initialize({}));

    // Act
    const bool ok = unit->upgrade();

    // Assert
    EXPECT_TRUE(ok);
    const QJsonObject after = readJsonFile(unit->configurationPath);
    const QJsonArray quick = after.value("QuickAccess").toObject().value("Items").toArray();
    ASSERT_GT(quick.size(), 7);
    bool hasLegacy = false;
    for (const QJsonValue &v : quick) {
        if (v.toObject().value("name").toString() == QString("projects"))
            hasLegacy = true;
    }
    EXPECT_TRUE(hasLegacy);
    EXPECT_NO_FATAL_FAILURE(unit->completed());
}

TEST_F(BookMarkCovTest, DoUpgrade_UnwritablePath_Fails)
{
    // Arrange
    unit->configurationPath = QDir::homePath() + "/missing-dir-xyz/sub/config.json";

    // Act
    const bool ok = unit->doUpgrade(QVariantList());

    // Assert
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// DefaultItemManager
// ---------------------------------------------------------------------------
TEST(DefaultItemManagerCovTest, Instance_IsSingleton)
{
    // Arrange / Act
    DefaultItemManager *first = DefaultItemManager::instance();
    DefaultItemManager *second = DefaultItemManager::instance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

TEST(DefaultItemManagerCovTest, InitDefaultItems_CoversStandardPathsInOrder)
{
    // Arrange / Act
    DefaultItemManager::instance()->initDefaultItems();
    const QList<BookmarkData> items = DefaultItemManager::instance()->defaultItemInitOrder();

    // Assert
    ASSERT_EQ(items.size(), 7);
    EXPECT_EQ(items.first().name, QString("Home"));
    EXPECT_EQ(items.last().name, QString("Downloads"));
    EXPECT_TRUE(items.first().isDefaultItem);
}

TEST(DefaultItemManagerCovTest, InitPreDefineItems_EmptyEnvironment_SafeNoop)
{
    // Arrange / Act
    EXPECT_NO_FATAL_FAILURE(DefaultItemManager::instance()->initPreDefineItems());
    const QList<BookmarkData> predef = DefaultItemManager::instance()->defaultPreDefInitOrder();

    // Assert: whatever the environment provides, every entry carries a valid url
    for (const BookmarkData &data : predef)
        EXPECT_TRUE(data.url.isValid() || data.url.toString().isEmpty());
}

// ---------------------------------------------------------------------------
// builtininterface.h free functions
// ---------------------------------------------------------------------------
TEST(BuiltinInterfaceCovTest, UpgradeConfigDir_PointsToSandboxManagerConfig)
{
    // Arrange / Act
    const QString dir = upgradeConfigDir();

    // Assert
    EXPECT_TRUE(dir.endsWith("deepin/dde-file-manager"));
    EXPECT_TRUE(dir.contains(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));
}

TEST(BuiltinInterfaceCovTest, IsNeedUpgrade_FlagFileControlsResult)
{
    // Arrange: no flag -> false
    QFile::remove(upgradeConfigDir() + "/" + kUpgradeFlag);
    EXPECT_FALSE(isNeedUpgrade());

    // Arrange: flag present + writable dir -> true
    QDir().mkpath(upgradeConfigDir());
    QFile flag(upgradeConfigDir() + "/" + kUpgradeFlag);
    ASSERT_TRUE(flag.open(QIODevice::WriteOnly));
    flag.close();

    // Act / Assert
    EXPECT_TRUE(isNeedUpgrade());
    QFile::remove(upgradeConfigDir() + "/" + kUpgradeFlag);
}

TEST(BuiltinInterfaceCovTest, TryUpgrade_MissingLibrary_ReturnsOne)
{
    // Arrange
    QMap<QString, QString> args { { "Desktop", "1" } };

    // Act
    const int ret = tryUpgrade(QDir::homePath() + "/no-such-lib-dfm-upgrade.so", args);

    // Assert
    EXPECT_EQ(ret, 1);
}

TEST(BuiltinInterfaceCovTest, TryUpgrade_RealLibraryWithInvalidArgs_MinusOne)
{
    // Arrange: load the actual upgrade library (built next to this test) and
    // call its entry point with empty args -> it rejects them with -1
    QMap<QString, QString> args;

    // Act
    const int ret = tryUpgrade(QString(UT_UPGRADE_LIB_PATH), args);

    // Assert
    EXPECT_EQ(ret, -1);
}

// ---------------------------------------------------------------------------
// CrashHandle::handleSignal
// ---------------------------------------------------------------------------
class CrashHandleCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        handle = new CrashHandle();
    }
    void TearDown() override
    {
        CrashHandle::unregSignal();
        delete handle;
        stub.clear();
    }

    CrashHandle *handle { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CrashHandleCovTest, HandleSignal_CreatesEscalatingCrashFlags)
{
    // Arrange: never re-raise the signal for real
    stub.set(&raise, +[](int) -> int { return 0; });
    const QString cacheDir = CrashHandle::upgradeCacheDir();
    QDir().mkpath(cacheDir);

    // Act: first crash writes flag0, second writes flag1
    CrashHandle::handleSignal(SIGSEGV);
    const bool firstFlag = QFile::exists(cacheDir + "/dfm-upgraded.crash.0");
    CrashHandle::handleSignal(SIGSEGV);
    const bool crashed = handle->isCrashed();

    // Assert
    EXPECT_TRUE(firstFlag);
    EXPECT_TRUE(crashed);

    // Act: clearCrash removes both flags
    handle->clearCrash();

    // Assert
    EXPECT_FALSE(handle->isCrashed());
}

// ---------------------------------------------------------------------------
// upgradeinterface.cpp entry points
// ---------------------------------------------------------------------------
class UpgradeInterfaceCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // default environment: flag file present so isNeedUpgrade() is true
        QDir().mkpath(upgradeConfigDir());
        flagPath = upgradeConfigDir() + "/" + kUpgradeFlag;
        QFile flag(flagPath);
        flag.open(QIODevice::WriteOnly);
        flag.close();
    }

    void TearDown() override
    {
        CrashHandle::unregSignal();
        QFile::remove(flagPath);
        stub.clear();
    }

    // make the dialog approve the upgrade without any UI and without /proc scans
    void stubDialogApprove()
    {
        stub.set_lamda(&ProcessDialog::queryProcess,
                       [](ProcessDialog *, const QString &) -> QList<int> {
                           __DBG_STUB_INVOKE__
                           return {};
                       });
    }

    // neutralize the shared-memory lock and the real upgrade chain
    void stubLockAndFactory()
    {
        stub.set_lamda(ADDR(UpgradeLocker, isLock),
                       [](UpgradeLocker *) -> bool {
                           __DBG_STUB_INVOKE__
                           return false;
                       });
        stub.set_lamda(ADDR(UpgradeFactory, previous),
                       [](UpgradeFactory *, const QMap<QString, QString> &) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(ADDR(UpgradeFactory, doUpgrade),
                       [](UpgradeFactory *) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(ADDR(UpgradeFactory, completed),
                       [](UpgradeFactory *) {
                           __DBG_STUB_INVOKE__
                       });
    }

    QString flagPath;
    stub_ext::StubExt stub;
};

TEST_F(UpgradeInterfaceCovTest, DoUpgrade_InvalidArgs_ReturnsMinusOne)
{
    // Arrange: neither Desktop nor FileManager given
    QMap<QString, QString> args { { "Whatever", "1" } };

    // Act
    const int ret = dfm_tools_upgrade_doUpgrade(args);

    // Assert: rejected before anything else happens
    EXPECT_EQ(ret, -1);
    EXPECT_TRUE(QFile::exists(flagPath));
}

TEST_F(UpgradeInterfaceCovTest, DoUpgrade_HappyPath_RemovesFlagFile)
{
    // Arrange
    QMap<QString, QString> args { { "Desktop", "1" } };
    stubDialogApprove();
    stubLockAndFactory();

    // Act
    const int ret = dfm_tools_upgrade_doUpgrade(args);

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(QFile::exists(flagPath));
}

TEST_F(UpgradeInterfaceCovTest, DoUpgrade_AlreadyCrashedOnce_ClearsState)
{
    // Arrange: pretend a previous run crashed twice -> give up gracefully
    stub.set_lamda(ADDR(CrashHandle, isCrashed),
                   [](CrashHandle *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    stub.set_lamda(ADDR(CrashHandle, clearCrash),
                   [](CrashHandle *) {
                       __DBG_STUB_INVOKE__
                   });
    QMap<QString, QString> args { { "FileManager", "1" } };

    // Act
    const int ret = dfm_tools_upgrade_doUpgrade(args);

    // Assert: aborted with success code and the flag file was removed
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(QFile::exists(flagPath));
}

TEST_F(UpgradeInterfaceCovTest, DoRestart_InvalidArgs_ReturnsMinusOne)
{
    // Arrange
    QMap<QString, QString> args;

    // Act
    const int ret = dfm_tools_upgrade_doRestart(args);

    // Assert
    EXPECT_EQ(ret, -1);
}

TEST_F(UpgradeInterfaceCovTest, DoRestart_ApprovedDialog_ReturnsZero)
{
    // Arrange
    QMap<QString, QString> args { { "FileManager", "1" } };
    stubDialogApprove();

    // Act
    const int ret = dfm_tools_upgrade_doRestart(args);

    // Assert
    EXPECT_EQ(ret, 0);
}
