// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "units/tagdbupgradeunit.h"
#include "units/beans/filetaginfo.h"
#include "units/beans/tagproperty.h"
#include "units/beans/oldfileproperty.h"
#include "units/beans/oldtagproperty.h"

#include <dfm-base/base/db/sqlitehandle.h>

#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

class TagDbUpgradeUnitTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        unit = new TagDbUpgradeUnit();
    }

    void TearDown() override
    {
        // the unit destructor deletes the handles it owns
        delete unit;
        unit = nullptr;
        dir.reset();
    }

    // create a regular file under the sandbox HOME so checkFileUrl can resolve it
    QString makeHomeFile(const QString &name)
    {
        const QString path = QDir::home().filePath(name);
        QFile f(path);
        EXPECT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("data");
        f.close();
        return path;
    }

    // create a sqlite db file with an old `tag_property` table and one row
    SqliteHandle *makeMainDb(const QString &name, const QString &tagName, const QString &tagColor)
    {
        const QString path = dir->filePath(name);
        auto *handle = new SqliteHandle(path);
        bool ok = handle->createTable<OldTagProperty>();
        EXPECT_TRUE(ok);
        if (!tagName.isEmpty()) {
            OldTagProperty bean;
            bean.setTagIndex(1);
            bean.setTagName(tagName);
            bean.setTagColor(tagColor);
            // customPK=true so the legacy integer key column is inserted too
            EXPECT_GE(handle->insert<OldTagProperty>(bean, true), 0);
        }
        return handle;
    }

    SqliteHandle *makeNewDb(const QString &name)
    {
        const QString path = dir->filePath(name);
        auto *handle = new SqliteHandle(path);
        EXPECT_TRUE(handle->createTable<TagProperty>(SqliteConstraint::primary("tagIndex"),
                                                     SqliteConstraint::autoIncreament("tagIndex"),
                                                     SqliteConstraint::unique("tagIndex")));
        EXPECT_TRUE(handle->createTable<FileTagInfo>(SqliteConstraint::primary("fileIndex"),
                                                     SqliteConstraint::autoIncreament("fileIndex"),
                                                     SqliteConstraint::unique("fileIndex")));
        return handle;
    }

    std::unique_ptr<QTemporaryDir> dir;
    TagDbUpgradeUnit *unit { nullptr };
};

TEST_F(TagDbUpgradeUnitTest, Name_And_Initialize_AlwaysSucceed)
{
    // Arrange
    QMap<QString, QString> args;

    // Act
    const QString name = unit->name();
    const bool inited = unit->initialize(args);

    // Assert
    EXPECT_EQ(name, QString("TagDbUpgradeUnit"));
    EXPECT_TRUE(inited);
}

TEST_F(TagDbUpgradeUnitTest, GetColorRGB_KnownAndUnknownColors_MappedOrEmpty)
{
    // Arrange
    const QString known = "Orange";
    const QString unknown = "NoSuchColor";

    // Act
    const QString mapped = unit->getColorRGB(known);
    const QString unmapped = unit->getColorRGB(unknown);

    // Assert
    EXPECT_EQ(mapped, QString("#ffa503"));
    EXPECT_TRUE(unmapped.isEmpty());
    EXPECT_EQ(unit->getColorRGB("Red"), QString("#ff1c49"));
    EXPECT_EQ(unit->getColorRGB("Gray"), QString("#cccccc"));
}

TEST_F(TagDbUpgradeUnitTest, CheckFileUrl_ExistAndMissingPaths_ResolvedOrEmpty)
{
    // Arrange
    const QString path = makeHomeFile("cov-exists.txt");
    const QString pseudoUrl = QString("xx/yy/") + QFileInfo(path).fileName();

    // Act
    const QString hit = unit->checkFileUrl(pseudoUrl);
    const QString tooShort = unit->checkFileUrl("onlyone");
    const QString missing = unit->checkFileUrl("aa/bb/definitely-missing-file-xyz");

    // Assert
    EXPECT_EQ(hit, path);
    EXPECT_TRUE(tooShort.isEmpty());
    EXPECT_TRUE(missing.isEmpty());
}

TEST_F(TagDbUpgradeUnitTest, ChechTable_MissingAndExistingTable_CreatedOrFound)
{
    // Arrange
    unit->newTagDbhandle = makeNewDb("chech.db");
    ASSERT_TRUE(unit->newTagDbhandle->dropTable<TagProperty>());

    // Act & Assert: table missing, newTable=false -> false; newTable=true -> created
    EXPECT_FALSE(unit->chechTable(unit->newTagDbhandle, "tag_property", false));
    EXPECT_TRUE(unit->chechTable(unit->newTagDbhandle, "tag_property", true));

    // Act & Assert: table now exists -> true regardless of flag
    EXPECT_TRUE(unit->chechTable(unit->newTagDbhandle, "tag_property", false));
    EXPECT_TRUE(unit->chechTable(unit->newTagDbhandle, "tag_property", true));
}

TEST_F(TagDbUpgradeUnitTest, CreateTableForNewDb_KnownAndUnknownTables_SuccessOrFailure)
{
    // Arrange
    unit->newTagDbhandle = makeNewDb("create.db");
    ASSERT_TRUE(unit->newTagDbhandle->dropTable<TagProperty>());
    ASSERT_TRUE(unit->newTagDbhandle->dropTable<FileTagInfo>());

    // Act
    const bool tagProp = unit->createTableForNewDb("tag_property");
    const bool fileTags = unit->createTableForNewDb("file_tags");
    const bool unknown = unit->createTableForNewDb("no_such_table");

    // Assert
    EXPECT_TRUE(tagProp);
    EXPECT_TRUE(fileTags);
    EXPECT_FALSE(unknown);
}

TEST_F(TagDbUpgradeUnitTest, UpgradeTagProperty_OldRowsMigrated_WithColorMapping)
{
    // Arrange
    unit->mainDbHandle = makeMainDb("main.db", "work", "Navy-blue");
    unit->newTagDbhandle = makeNewDb("new.db");

    // Act
    const bool ok = unit->upgradeTagProperty();

    // Assert
    EXPECT_TRUE(ok);
    const auto beans = unit->newTagDbhandle->query<TagProperty>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans.at(0)->getTagName(), QString("work"));
    EXPECT_EQ(beans.at(0)->getTagColor(), QString("#3468ff"));
    EXPECT_EQ(beans.at(0)->getAmbiguity(), 1);
}

TEST_F(TagDbUpgradeUnitTest, UpgradeTagProperty_NoOldRows_SucceedsWithoutInsert)
{
    // Arrange
    unit->mainDbHandle = makeMainDb("empty-main.db", QString(), QString());
    unit->newTagDbhandle = makeNewDb("empty-new.db");

    // Act
    const bool ok = unit->upgradeTagProperty();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(unit->newTagDbhandle->query<TagProperty>().toBeans().isEmpty());
}

TEST_F(TagDbUpgradeUnitTest, UpgradeFileTag_OldRowsMigrated_WhenFileExists)
{
    // Arrange
    const QString homeFile = makeHomeFile("cov-filetag.txt");

    const QString deepinPath = dir->filePath("deepin.db");
    unit->deepinDbHandle = new SqliteHandle(deepinPath);
    ASSERT_TRUE(unit->deepinDbHandle->createTable<OldFileProperty>());
    OldFileProperty valid;
    valid.setFilePath(QString("xx/yy/") + QFileInfo(homeFile).fileName());
    valid.setTag("important");
    ASSERT_GE(unit->deepinDbHandle->insert<OldFileProperty>(valid, true), 0);
    OldFileProperty invalidPath;
    invalidPath.setFilePath("aa/bb/gone-file-xyz");
    invalidPath.setTag("lost");
    ASSERT_GE(unit->deepinDbHandle->insert<OldFileProperty>(invalidPath, true), 0);

    unit->newTagDbhandle = makeNewDb("ft-new.db");

    // Act
    const bool ok = unit->upgradeFileTag();

    // Assert
    EXPECT_TRUE(ok);
    const auto beans = unit->newTagDbhandle->query<FileTagInfo>().toBeans();
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans.at(0)->getTagName(), QString("important"));
    EXPECT_EQ(beans.at(0)->getFilePath(), homeFile);
}

TEST_F(TagDbUpgradeUnitTest, UpgradeTagDb_And_UpgradeData_FullMigrationSucceed)
{
    // Arrange
    const QString homeFile = makeHomeFile("cov-full.txt");

    unit->mainDbHandle = makeMainDb("m.db", "colorTag", "Yellow");

    const QString deepinPath = dir->filePath("d.db");
    unit->deepinDbHandle = new SqliteHandle(deepinPath);
    ASSERT_TRUE(unit->deepinDbHandle->createTable<OldFileProperty>());
    OldFileProperty row;
    row.setFilePath(QString("xx/yy/") + QFileInfo(homeFile).fileName());
    row.setTag("colorTag");
    ASSERT_GE(unit->deepinDbHandle->insert<OldFileProperty>(row, true), 0);

    unit->newTagDbhandle = makeNewDb("n.db");

    // Act
    const bool upgraded = unit->upgradeTagDb();
    const bool upgradedData = unit->upgradeData();

    // Assert: each migration run inserts one row per legacy record; running
    // both upgradeTagDb() and upgradeData() migrates the same data twice
    EXPECT_TRUE(upgraded);
    EXPECT_TRUE(upgradedData);
    EXPECT_EQ(unit->newTagDbhandle->query<TagProperty>().toBeans().size(), 2);
    EXPECT_EQ(unit->newTagDbhandle->query<FileTagInfo>().toBeans().size(), 2);
}

TEST_F(TagDbUpgradeUnitTest, CheckOldDatabase_SystemShareDbAbsent_ReturnsFalse)
{
    // Arrange / Act
    const bool exists = unit->checkOldDatabase();

    // Assert: the old databases live in the (read-only) application share
    // directory which does not exist in the sandbox, so nothing to upgrade
    EXPECT_FALSE(exists);
}

TEST_F(TagDbUpgradeUnitTest, Upgrade_NoOldDatabase_ReturnsFalseAfterNewDbChecked)
{
    // Arrange / Act
    const bool ok = unit->upgrade();

    // Assert: checkNewDatabase succeeds inside the sandbox HOME (creates
    // dfmruntime.db), but checkOldDatabase fails -> upgrade gives up
    EXPECT_FALSE(ok);
    const QString configDb = QDir::homePath() + "/.config/deepin/dde-file-manager/database/dfmruntime.db";
    EXPECT_TRUE(QFile::exists(configDb));
}
