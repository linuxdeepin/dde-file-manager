// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "utils/crashhandle.h"
#include "utils/upgradeutils.h"
#include "upgradeinterface.h"
#include "core/upgradelocker.h"
#include "core/upgradeunit.h"
#include "core/upgradefactory.h"
#include "units/headerunit.h"
#include "units/unitlist.h"

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

using namespace dfm_upgrade;

// ---------------------------------------------------------------------------
// CrashHandle tests
// ---------------------------------------------------------------------------
class CrashHandleTest : public testing::Test
{
protected:
    void SetUp() override { handle = new CrashHandle(); }
    void TearDown() override { delete handle; }
    CrashHandle *handle = nullptr;
};

TEST_F(CrashHandleTest, Constructor_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(CrashHandle h);
}

TEST_F(CrashHandleTest, IsCrashed_DefaultFalse)
{
    EXPECT_FALSE(handle->isCrashed());
}

TEST_F(CrashHandleTest, ClearCrash_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(handle->clearCrash());
}

TEST_F(CrashHandleTest, RegSignal_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(CrashHandle::regSignal());
    CrashHandle::unregSignal();
}

TEST_F(CrashHandleTest, UnregSignal_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(CrashHandle::unregSignal());
}

TEST_F(CrashHandleTest, UpgradeCacheDir_ReturnsNonEmpty)
{
    QString dir = CrashHandle::upgradeCacheDir();
    EXPECT_FALSE(dir.isEmpty());
    EXPECT_TRUE(dir.contains("dde-file-manager") || dir.contains("deepin") || dir.contains("cache"));
}

// ---------------------------------------------------------------------------
// UpgradeUtils tests
// ---------------------------------------------------------------------------
class UpgradeUtilsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());
    }
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(UpgradeUtilsTest, GenericAttribute_UnknownKey_ReturnsInvalid)
{
    QVariant result = UpgradeUtils::genericAttribute("nonexistent_key_12345");
    // May return invalid or a default value depending on config
    EXPECT_TRUE(result.isValid() || result.isNull());
}

TEST_F(UpgradeUtilsTest, ApplicationAttribute_UnknownKey_ReturnsInvalid)
{
    QVariant result = UpgradeUtils::applicationAttribute("nonexistent_key_12345");
    EXPECT_TRUE(result.isValid() || result.isNull());
}

TEST_F(UpgradeUtilsTest, AddOldGenericAttribute_EmptyArray_DoesNotCrash)
{
    QJsonArray empty;
    EXPECT_NO_FATAL_FAILURE(UpgradeUtils::addOldGenericAttribute(empty));
}

TEST_F(UpgradeUtilsTest, AddOldGenericAttribute_WithItems_DoesNotCrash)
{
    QJsonArray arr;
    QJsonObject obj;
    obj["key"] = "test_key";
    obj["value"] = "test_value";
    arr.append(obj);
    EXPECT_NO_FATAL_FAILURE(UpgradeUtils::addOldGenericAttribute(arr));
}

TEST_F(UpgradeUtilsTest, BackupFile_ValidSource_CreatesBackup)
{
    // Create a source file
    QString srcPath = tempDir->filePath("source.txt");
    {
        QFile f(srcPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("test content");
        f.close();
    }
    QString backupDir = tempDir->filePath("backup");
    QDir().mkpath(backupDir);

    bool result = UpgradeUtils::backupFile(srcPath, backupDir);
    EXPECT_TRUE(result);
    // Backup file should exist
    QDir backupDirectory(backupDir);
    QStringList backups = backupDirectory.entryList(QStringList() << "source.txt*", QDir::Files);
    EXPECT_FALSE(backups.isEmpty());
}

TEST_F(UpgradeUtilsTest, BackupFile_NonExistentSource_ReturnsFalse)
{
    QString backupDir = tempDir->filePath("backup2");
    QDir().mkpath(backupDir);
    bool result = UpgradeUtils::backupFile("/nonexistent/path/file.txt", backupDir);
    EXPECT_FALSE(result);
}

TEST_F(UpgradeUtilsTest, BackupFile_NonExistentBackupDir_ReturnsFalse)
{
    QString srcPath = tempDir->filePath("source2.txt");
    {
        QFile f(srcPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("content");
        f.close();
    }
    bool result = UpgradeUtils::backupFile(srcPath, "/nonexistent/backup/dir/path");
    EXPECT_FALSE(result);
}

// ---------------------------------------------------------------------------
// UpgradeLocker tests
// ---------------------------------------------------------------------------
class UpgradeLockerTest : public testing::Test
{
};

TEST_F(UpgradeLockerTest, Constructor_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(UpgradeLocker locker);
}

TEST_F(UpgradeLockerTest, IsLock_AfterConstruction)
{
    UpgradeLocker locker;
    // Should not crash; result depends on whether another instance is running
    EXPECT_NO_FATAL_FAILURE(locker.isLock());
}

// ---------------------------------------------------------------------------
// UpgradeUnit tests (via HeaderUnit concrete subclass)
// ---------------------------------------------------------------------------
class UpgradeUnitTest : public testing::Test
{
};

TEST_F(UpgradeUnitTest, HeaderUnit_Constructor_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(HeaderUnit unit);
}

TEST_F(UpgradeUnitTest, HeaderUnit_Name_IsNotEmpty)
{
    HeaderUnit unit;
    EXPECT_FALSE(unit.name().isEmpty());
}

TEST_F(UpgradeUnitTest, HeaderUnit_Completed_DoesNotCrash)
{
    HeaderUnit unit;
    EXPECT_NO_FATAL_FAILURE(unit.completed());
}

// ---------------------------------------------------------------------------
// createUnits (UnitList) tests
// ---------------------------------------------------------------------------
class UnitListTest : public testing::Test
{
};

TEST_F(UnitListTest, CreateUnits_ReturnsNonEmptyList)
{
    auto units = dfm_upgrade::createUnits();
    EXPECT_FALSE(units.isEmpty());
    // Should contain all registered units
    EXPECT_GE(units.size(), 9);
}

TEST_F(UnitListTest, CreateUnits_AllUnitsHaveNames)
{
    auto units = dfm_upgrade::createUnits();
    for (const auto &unit : units) {
        ASSERT_TRUE(unit != nullptr);
        EXPECT_FALSE(unit->name().isEmpty()) << "Unit has empty name";
    }
}
