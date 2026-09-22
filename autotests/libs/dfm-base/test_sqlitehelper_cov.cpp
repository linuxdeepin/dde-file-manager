// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqlitehelper_cov.cpp
 * @brief Coverage-focused tests for the db/sqlitehelper.h, db/sqlitehandle.h
 *        and db/sqlitequeryable.h template instantiations named in the
 *        ut-dfm-base gap list.  The bean type parameters (OldTagProperty,
 *        OldFileProperty, SqliteMaster, dfm_upgrade::TagProperty/FileTagInfo/
 *        VirtualEntryData, dfmplugin_smbbrowser::VirtualEntryData) are
 *        provided by ut_covbeans.h with matching namespace/class names.
 *
 * Covered gap-list functions -> case mapping:
 *   SqliteHelper::tableName<T>            -> TableName_CovBeans_ReturnClassInfo
 *   SqliteHelper::fieldNames<T> (+lambda) -> FieldNames_CovBeans_ListAllProperties
 *   SqliteHelper::visit<T>                -> Visit_CovBeans_InvokeCallbackPerProperty
 *   SqliteHelper::fieldTypesMap<T>(+lmbd) -> FieldTypesMap_UpgradeBeans_MapEveryField
 *   SerializationHelper::deserialize<T>   -> Deserialize_PropertyMap_AppliedToBean
 *   Expression::Field<SqliteMaster>       -> Field_SqliteMaster_MakesFieldExpr
 *   SqliteHandle::createTable<T>(+lmbd)   -> CreateTable_ConstrainedBeans_TableCreated
 *   SqliteHandle::dropTable<smb VED>      -> DropTable_SmbVed_TableRemoved
 *   SqliteHandle::insert<T> (+2 lambdas)  -> Insert_NewBeans_RowIdReturned
 *   SqliteHandle::update<smb VED>         -> Update_SmbVed_RowModified
 *   SqliteHandle::query<Old*>             -> Query_OldBeans_RowsReturned
 *   SqliteHandle::remove<smb VED>(x2)     -> Remove_SmbVed_RowsDeleted
 *   SqliteQueryable ctor/where/toBeans/
 *   toMaps/getFromSql/getLimit/queryToMaps-> Query_OldBeans_RowsReturned,
 *                                             Queryable_SqlBuilders_Assembled
 *
 * Branch notes (from source):
 *   - SqliteHandle::insert: customPK true/false (start index 0/1).
 *   - SqliteHandle::remove(entity): first-field where clause.
 *   - SqliteQueryable::getLimit: empty orderBy / take+skip present.
 *   - createTable: trailing-comma chop only when no table-level constraint.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>
#include <dfm-base/base/db/sqlitequeryable.h>

#include <QHash>
#include <QList>
#include <QMetaProperty>
#include <QSharedPointer>
#include <QStringList>
#include <QTemporaryDir>
#include <QVariant>
#include <QVariantMap>

#include "ut_covbeans.h"

using namespace dfmbase;
using namespace dfmbase::Expression;

class UT_SqliteHelperCov : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dbPath = tmpDir.path() + "/ut_cov.db";
    }

    // Small helper: count rows of `table` via the handle itself.
    static int rowCount(SqliteHandle &handle, const QString &table)
    {
        int n = -1;
        handle.excute("SELECT COUNT(*) FROM " + table + ";",
                      [&n](QSqlQuery *query) {
                          if (query->next())
                              n = query->value(0).toInt();
                      });
        return n;
    }

    QTemporaryDir tmpDir;
    QString dbPath;
};

// ---------------------------------------------------------------------------
// SqliteHelper statics
// ---------------------------------------------------------------------------

TEST_F(UT_SqliteHelperCov, TableName_CovBeans_ReturnClassInfo)
{
    // Arrange
    // Act — resolve the table name of every coverage bean.

    // Assert — one assert per bean instantiation.
    EXPECT_EQ(SqliteHelper::tableName<OldTagProperty>().toStdString(), "ut_cov_old_tag_property");
    EXPECT_EQ(SqliteHelper::tableName<OldFileProperty>().toStdString(), "ut_cov_old_file_property");
    EXPECT_EQ(SqliteHelper::tableName<SqliteMaster>().toStdString(), "sqlite_master");
    EXPECT_EQ(SqliteHelper::tableName<dfm_upgrade::TagProperty>().toStdString(), "ut_cov_tag_property");
    EXPECT_EQ(SqliteHelper::tableName<dfm_upgrade::FileTagInfo>().toStdString(), "ut_cov_file_tag_info");
    EXPECT_EQ(SqliteHelper::tableName<dfm_upgrade::VirtualEntryData>().toStdString(), "ut_cov_upgrade_ved");
}

TEST_F(UT_SqliteHelperCov, FieldNames_CovBeans_ListAllProperties)
{
    // Arrange
    // Act
    const QStringList oldTag = SqliteHelper::fieldNames<OldTagProperty>();

    // Assert
    ASSERT_EQ(oldTag.size(), 3);
    EXPECT_EQ(oldTag.first().toStdString(), "tag_index");
    EXPECT_EQ(oldTag.last().toStdString(), "tag_color");

    EXPECT_EQ(SqliteHelper::fieldNames<SqliteMaster>().size(), 5);
    EXPECT_EQ(SqliteHelper::fieldNames<OldFileProperty>().size(), 3);
    EXPECT_EQ(SqliteHelper::fieldNames<dfm_upgrade::TagProperty>().size(), 3);
    EXPECT_EQ(SqliteHelper::fieldNames<dfm_upgrade::FileTagInfo>().size(), 3);
    EXPECT_EQ(SqliteHelper::fieldNames<dfm_upgrade::VirtualEntryData>().size(), 2);
    EXPECT_EQ(SqliteHelper::fieldNames<dfmplugin_smbbrowser::VirtualEntryData>().size(), 3);
}

TEST_F(UT_SqliteHelperCov, Visit_CovBeans_InvokeCallbackPerProperty)
{
    // Arrange
    int oldTagCount = 0;
    int masterCount = 0;
    int upgradeCount = 0;

    // Act
    SqliteHelper::visit<OldTagProperty>([&oldTagCount](const QMetaProperty &) { ++oldTagCount; });
    SqliteHelper::visit<SqliteMaster>([&masterCount](const QMetaProperty &) { ++masterCount; });
    SqliteHelper::visit<dfm_upgrade::VirtualEntryData>(
            [&upgradeCount](const QMetaProperty &) { ++upgradeCount; });
    SqliteHelper::visit<dfm_upgrade::FileTagInfo>([](const QMetaProperty &) {});
    SqliteHelper::visit<dfm_upgrade::TagProperty>([](const QMetaProperty &) {});
    SqliteHelper::visit<OldFileProperty>([](const QMetaProperty &) {});

    // Assert — objectName + declared properties are all visited.
    EXPECT_EQ(oldTagCount, 4);
    EXPECT_EQ(masterCount, 6);
    EXPECT_EQ(upgradeCount, 3);
}

TEST_F(UT_SqliteHelperCov, FieldTypesMap_UpgradeBeans_MapEveryField)
{
    // Arrange
    const QStringList fields = SqliteHelper::fieldNames<dfm_upgrade::FileTagInfo>();
    QHash<QString, QString> fileTagTypes;
    QHash<QString, QString> tagTypes;
    QHash<QString, QString> upgradeVedTypes;
    QHash<QString, QString> smbVedTypes;

    // Act
    SqliteHelper::fieldTypesMap<dfm_upgrade::FileTagInfo>(fields, &fileTagTypes);
    SqliteHelper::fieldTypesMap<dfm_upgrade::TagProperty>(
            SqliteHelper::fieldNames<dfm_upgrade::TagProperty>(), &tagTypes);
    SqliteHelper::fieldTypesMap<dfm_upgrade::VirtualEntryData>(
            SqliteHelper::fieldNames<dfm_upgrade::VirtualEntryData>(), &upgradeVedTypes);
    SqliteHelper::fieldTypesMap<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteHelper::fieldNames<dfmplugin_smbbrowser::VirtualEntryData>(), &smbVedTypes);

    // Assert
    ASSERT_EQ(fileTagTypes.size(), 3);
    EXPECT_EQ(fileTagTypes.value("file_index").toStdString(), " INTEGER NOT NULL");
    EXPECT_EQ(fileTagTypes.value("file_path").toStdString(), " TEXT NOT NULL");
    EXPECT_EQ(tagTypes.size(), 3);
    EXPECT_EQ(upgradeVedTypes.size(), 2);
    EXPECT_EQ(smbVedTypes.size(), 3);
}

TEST_F(UT_SqliteHelperCov, Deserialize_PropertyMap_AppliedToBean)
{
    // Arrange
    QVariantMap tagMap { { "tag_index", 7 },
                         { "tag_name", QString("important") },
                         { "tag_color", QString("#ff0000") } };
    QVariantMap masterMap { { "type", QString("table") },
                            { "name", QString("mytable") },
                            { "rootpage", 3 } };

    // Act
    OldTagProperty *tag = SerializationHelper::deserialize<OldTagProperty>(tagMap);
    OldFileProperty *file = SerializationHelper::deserialize<OldFileProperty>(
            QVariantMap { { "property_index", 2 }, { "file_path", QString("/a") } });
    SqliteMaster *master = SerializationHelper::deserialize<SqliteMaster>(masterMap);
    auto *smbVed = SerializationHelper::deserialize<dfmplugin_smbbrowser::VirtualEntryData>(
            QVariantMap { { "ved_index", 9 }, { "ved_name", QString("smb") } });

    // Assert
    ASSERT_TRUE(tag != nullptr);
    EXPECT_EQ(tag->tagIndex(), 7);
    EXPECT_EQ(tag->tagName().toStdString(), "important");
    ASSERT_TRUE(file != nullptr);
    EXPECT_EQ(file->propertyIndex(), 2);
    ASSERT_TRUE(master != nullptr);
    EXPECT_EQ(master->getName().toStdString(), "mytable");
    EXPECT_EQ(master->getRootpage(), 3);
    ASSERT_TRUE(smbVed != nullptr);
    EXPECT_EQ(smbVed->vedIndex(), 9);

    delete tag;
    delete file;
    delete master;
    delete smbVed;
}

TEST_F(UT_SqliteHelperCov, Field_SqliteMaster_MakesFieldExpr)
{
    // Arrange
    // Act
    ExprField field = Field<SqliteMaster>("name");

    // Assert — Field<T>() wraps the raw column name into an expression field.
    EXPECT_EQ(field.fieldName.toStdString(), "name");
    const Expr expr = (field == QVariant(QString("mytable")));
    EXPECT_EQ(expr.toString().toStdString(), "name='mytable'");
}

// ---------------------------------------------------------------------------
// SqliteHandle CRUD (real on-disk sqlite database)
// ---------------------------------------------------------------------------

TEST_F(UT_SqliteHelperCov, CreateTable_ConstrainedBeans_TableCreated)
{
    // Arrange
    SqliteHandle handle(dbPath);

    // Act — constraint arity mirrors the gap-list instantiations.
    const bool tagOk = handle.createTable<dfm_upgrade::TagProperty>(
            SqliteConstraint::primary("tag_index"),
            SqliteConstraint::autoIncreament("tag_index"),
            SqliteConstraint::unique("tag_name"));
    const bool upgradeVedOk = handle.createTable<dfm_upgrade::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index"));
    const bool fileTagOk = handle.createTable<dfm_upgrade::FileTagInfo>(
            SqliteConstraint::primary("file_index"),
            SqliteConstraint::autoIncreament("file_index"),
            SqliteConstraint::unique("file_path"));
    const bool smbOk = handle.createTable<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index"));

    // Assert (sqlite_master also lists UNIQUE auto-indexes, so >= 4 tables)
    EXPECT_TRUE(tagOk);
    EXPECT_TRUE(upgradeVedOk);
    EXPECT_TRUE(fileTagOk);
    ASSERT_TRUE(smbOk);
    const QString createSql = handle.lastQuery();
    EXPECT_GE(rowCount(handle, "sqlite_master"), 4);
    EXPECT_TRUE(createSql.contains("CREATE TABLE IF NOT EXISTS ut_cov_smb_ved"));
}

TEST_F(UT_SqliteHelperCov, DropTable_SmbVed_TableRemoved)
{
    // Arrange
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteConstraint::primary("ved_index")));
    ASSERT_EQ(rowCount(handle, "sqlite_master"), 1);

    // Act
    const bool dropped = handle.dropTable<dfmplugin_smbbrowser::VirtualEntryData>();
    const bool droppedAgain = handle.dropTable<dfmplugin_smbbrowser::VirtualEntryData>();

    // Assert
    EXPECT_TRUE(dropped);
    EXPECT_FALSE(droppedAgain);
    EXPECT_EQ(rowCount(handle, "ut_cov_smb_ved"), -1);
}

TEST_F(UT_SqliteHelperCov, Insert_NewBeans_RowIdReturned)
{
    // Arrange
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<dfm_upgrade::TagProperty>(
            SqliteConstraint::primary("tag_index"),
            SqliteConstraint::autoIncreament("tag_index"),
            SqliteConstraint::unique("tag_name")));
    ASSERT_TRUE(handle.createTable<dfm_upgrade::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index")));
    ASSERT_TRUE(handle.createTable<dfm_upgrade::FileTagInfo>(
            SqliteConstraint::primary("file_index"),
            SqliteConstraint::autoIncreament("file_index")));
    ASSERT_TRUE(handle.createTable<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index")));

    dfm_upgrade::TagProperty tag;
    tag.setTagName("red");
    dfm_upgrade::VirtualEntryData upgradeVed;
    upgradeVed.setVedDisplay("display");
    dfm_upgrade::FileTagInfo fileTag;
    fileTag.setFilePath("/tmp/x");
    fileTag.setTagName("red");
    dfmplugin_smbbrowser::VirtualEntryData smbVed;
    smbVed.setVedIndex(7);
    smbVed.setVedName("share");
    smbVed.setVedType("dir");

    // Act — default path skips the PK column, customPK=true writes it.
    const int tagId = handle.insert<dfm_upgrade::TagProperty>(tag);
    const int upgradeVedId = handle.insert<dfm_upgrade::VirtualEntryData>(upgradeVed);
    const int fileTagId = handle.insert<dfm_upgrade::FileTagInfo>(fileTag);
    const int smbId = handle.insert<dfmplugin_smbbrowser::VirtualEntryData>(smbVed, true);

    // Assert
    EXPECT_EQ(tagId, 1);
    EXPECT_EQ(upgradeVedId, 1);
    EXPECT_EQ(fileTagId, 1);
    EXPECT_EQ(smbId, 7);
    EXPECT_EQ(rowCount(handle, "ut_cov_tag_property"), 1);
    EXPECT_EQ(rowCount(handle, "ut_cov_smb_ved"), 1);
}

TEST_F(UT_SqliteHelperCov, Update_SmbVed_RowModified)
{
    // Arrange
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index")));
    dfmplugin_smbbrowser::VirtualEntryData ved;
    ved.setVedName("old");
    ASSERT_EQ(handle.insert<dfmplugin_smbbrowser::VirtualEntryData>(ved), 1);

    QString storedName;
    auto readName = [&storedName](QSqlQuery *query) {
        if (query->next())
            storedName = query->value(0).toString();
    };

    // Act
    const bool updated = handle.update<dfmplugin_smbbrowser::VirtualEntryData>(
            Field<dfmplugin_smbbrowser::VirtualEntryData>("ved_name") = QVariant(QString("new")),
            Field<dfmplugin_smbbrowser::VirtualEntryData>("ved_index") == QVariant(1));
    const QString updateSql = handle.lastQuery();
    ASSERT_TRUE(handle.excute("SELECT ved_name FROM ut_cov_smb_ved WHERE ved_index=1;", readName));

    // Assert
    EXPECT_TRUE(updated);
    EXPECT_EQ(storedName.toStdString(), "new");
    EXPECT_EQ(updateSql.toStdString().substr(0, 6), "UPDATE");
}

TEST_F(UT_SqliteHelperCov, Remove_SmbVed_RowsDeleted)
{
    // Arrange
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<dfmplugin_smbbrowser::VirtualEntryData>(
            SqliteConstraint::primary("ved_index"),
            SqliteConstraint::autoIncreament("ved_index")));
    dfmplugin_smbbrowser::VirtualEntryData first;
    first.setVedName("first");
    dfmplugin_smbbrowser::VirtualEntryData second;
    second.setVedName("second");
    const int firstId = handle.insert<dfmplugin_smbbrowser::VirtualEntryData>(first);
    const int secondId = handle.insert<dfmplugin_smbbrowser::VirtualEntryData>(second);
    // Sync the entity PK with the auto-incremented row id before removing.
    first.setVedIndex(firstId);
    second.setVedIndex(secondId);
    ASSERT_EQ(rowCount(handle, "ut_cov_smb_ved"), 2);

    // Act — entity overload removes by first field (PK), expression overload
    // removes by an explicit where clause.
    const bool removedByEntity = handle.remove<dfmplugin_smbbrowser::VirtualEntryData>(first);
    const bool removedByExpr = handle.remove<dfmplugin_smbbrowser::VirtualEntryData>(
            Field<dfmplugin_smbbrowser::VirtualEntryData>("ved_index") == QVariant(secondId));

    // Assert
    const QString deleteSql = handle.lastQuery();
    EXPECT_TRUE(removedByEntity);
    EXPECT_TRUE(removedByExpr);
    EXPECT_EQ(rowCount(handle, "ut_cov_smb_ved"), 0);
    EXPECT_TRUE(deleteSql.contains("DELETE FROM ut_cov_smb_ved"));
}

TEST_F(UT_SqliteHelperCov, Query_OldBeans_RowsReturned)
{
    // Arrange — old-style tables populated with raw SQL.
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldTagProperty>(SqliteConstraint::primary("tag_index")));
    ASSERT_TRUE(handle.createTable<OldFileProperty>(SqliteConstraint::primary("property_index")));
    ASSERT_TRUE(handle.excute("INSERT INTO ut_cov_old_tag_property(tag_index,tag_name,tag_color)"
                              " VALUES (1,'n1','c1'),(2,'n2','c2');"));
    ASSERT_TRUE(handle.excute("INSERT INTO ut_cov_old_file_property(property_index,file_path,file_property)"
                              " VALUES (1,'/f1','p1');"));
    ASSERT_GE(rowCount(handle, "sqlite_master"), 2);

    // Act
    const QList<QSharedPointer<OldTagProperty>> tags =
            handle.query<OldTagProperty>()
                    .where(Field<OldTagProperty>("tag_index") == QVariant(1))
                    .toBeans();
    const QList<QVariantMap> fileMaps = handle.query<OldFileProperty>().toMaps();
    const QList<QSharedPointer<SqliteMaster>> masters =
            handle.query<SqliteMaster>()
                    .where(Field<SqliteMaster>("type") == QVariant(QString("table")))
                    .toBeans();
    const QList<QVariantMap> masterMaps = handle.query<SqliteMaster>().toMaps();

    // Assert
    ASSERT_EQ(tags.size(), 1);
    EXPECT_EQ(tags.first()->tagName().toStdString(), "n1");
    EXPECT_EQ(tags.first()->tagColor().toStdString(), "c1");
    ASSERT_EQ(fileMaps.size(), 1);
    EXPECT_EQ(fileMaps.first().value("file_path").toString().toStdString(), "/f1");
    ASSERT_GE(masters.size(), 2);
    EXPECT_FALSE(masters.first()->getName().isEmpty());
    EXPECT_EQ(masterMaps.size(), masters.size());
}

TEST_F(UT_SqliteHelperCov, Queryable_SqlBuilders_Assembled)
{
    // Arrange — full five-argument constructor + builder chain.
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<OldTagProperty>(SqliteConstraint::primary("tag_index")));
    ASSERT_TRUE(handle.excute("INSERT INTO ut_cov_old_tag_property(tag_index,tag_name,tag_color)"
                              " VALUES (1,'n1','c1'),(2,'n2','c2');"));

    const SqliteQueryable<OldTagProperty> masterLike(
            dbPath, " FROM sqlite_master", "SELECT ", "*", " WHERE type='table'");
    SqliteQueryable<OldTagProperty> q(dbPath, " FROM ut_cov_old_tag_property");

    // Act
    const QString masterFromSql = masterLike.getFromSql();
    const QString masterLimit = masterLike.getLimit();
    q.orderBy(Field<OldTagProperty>("tag_index")).take(1).skip(0);
    const QString limitSql = q.getLimit();
    const QList<QVariantMap> oneRow = q.toMaps();
    const QList<QSharedPointer<OldTagProperty>> beans = q.toBeans();

    // Assert
    EXPECT_EQ(masterFromSql.toStdString(), " FROM sqlite_master WHERE type='table'");
    EXPECT_TRUE(masterLimit.isEmpty());
    EXPECT_EQ(limitSql.toStdString(), " ORDER BY tag_index LIMIT 1 OFFSET 0");
    ASSERT_EQ(oneRow.size(), 1);
    EXPECT_EQ(oneRow.first().value("tag_name").toString().toStdString(), "n1");
    ASSERT_EQ(beans.size(), 1);
    EXPECT_EQ(beans.first()->tagIndex(), 1);
}
