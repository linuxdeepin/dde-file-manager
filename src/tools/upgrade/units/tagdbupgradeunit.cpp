// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbupgradeunit.h"
#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"
#include "beans/oldfileproperty.h"
#include "beans/oldtagproperty.h"

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/db/sqliteconnectionpool.h"

#include <dfm-io/dfmio_utils.h>

static constexpr char kTagOldDbName1[] = ".__main.db";
static constexpr char kTagOldDb1TableTagProperty[] = "tag_property";

static constexpr char kTagOldDbName2[] = ".__deepin.db";
static constexpr char kTagOldDb2TableFileProperty[] = "file_property";
static constexpr char kTagOldDb2TableTagWithFile[] = "tag_with_file";

static constexpr char kTagNewDbName[] = ".__tag.db";
static constexpr char kTagNewTableFileTags[] = "file_tags";
static constexpr char kTagNewTableTagProperty[] = "tag_property";

static constexpr char kTagDbTableExistCheckSql[] = "SELECT count(*) FROM sqlite_master WHERE type=\"table\" AND name = \"%1\";";
static constexpr char kTagNewTableTagPropertyCreateSql[] = "CREATE TABLE `tag_property` ( `tagIndex` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                                           "`tagName` TEXT NOT NULL, `tagColor` TEXT NOT NULL, `ambiguity` INTEGER NOT NULL, `future` TEXT NOT NULL )";
static constexpr char kTagNewTableFileTagsCreateSql[] = "CREATE TABLE `file_tags` ( `fileIndex` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                                        "`filePath` TEXT NOT NULL, `tagName` TEXT NOT NULL, `tagOrder` INTEGER NOT NULL, `future` TEXT NOT NULL )";

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

TagDbUpgradeUnit::TagDbUpgradeUnit()
{
    tableInfos.insert(kTagNewTableTagProperty, kTagNewTableTagPropertyCreateSql);
    tableInfos.insert(kTagNewTableFileTags, kTagNewTableFileTagsCreateSql);
}

QString dfm_upgrade::TagDbUpgradeUnit::name()
{
    return "TagDbUpgradeUnit";
}

bool dfm_upgrade::TagDbUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args);
    return true;
}

bool dfm_upgrade::TagDbUpgradeUnit::upgrade()
{
    // check db
    // if checkNewDatabase return false, it's mean that we unable to upgrade
    if (!checkNewDatabase())
        return false;

    // if checkOldDatabase return false mean that we don't need upgrade
    if (!checkOldDatabase())
        return false;

    return upgradeTagDb();
}

bool TagDbUpgradeUnit::upgradeTagDb()
{
    // upgrade tag_property
    if (!upgradeTagProperty())
        return false;

    // upgrade file_tags
    if (!upgradeFileTag())
        return false;

    return true;
}

bool TagDbUpgradeUnit::checkDatabaseFile(const QString &dbPath)
{
    QString errString;
    auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(dbPath), false, &errString);
    return fileInfo ? true : false;
}

bool TagDbUpgradeUnit::chechTable(const QSqlDatabase &db, const QString &tableName, bool newTable)
{
    auto sqlStr = QString(kTagDbTableExistCheckSql).arg(tableName);
    QSqlQuery query { db };
    query.exec(sqlStr);

    if (!newTable)
        return query.next();

    return createTableForNewDb(db, tableName);
}

bool TagDbUpgradeUnit::createTableForNewDb(const QSqlDatabase &db, const QString &tableName)
{
    if (db.isValid() || db.isOpenError() || tableName.isEmpty())
        return false;

    const auto createSql = tableInfos.value(tableName);
    if (createSql.isEmpty()) {
        return false;
    }

    QSqlQuery query { db };
    return query.exec(createSql);
}

bool TagDbUpgradeUnit::upgradeData()
{
    // upgrade db
    if (!upgradeTagProperty())
        return false;

    if (!upgradeFileTag())
        return false;

    return true;
}

bool TagDbUpgradeUnit::upgradeTagProperty()
{
    // read old table
    const auto &tagPropertyBean = mainDbHandle->query<OldTagProperty>().toBeans();
    if (tagPropertyBean.isEmpty())
        return true;

    QVariantMap tagPropertyMap;
    for (auto &bean : tagPropertyBean) {
        TagProperty temp;
        temp.setTagName(bean->getTagName());
        temp.setTagColor(bean->getTagColor());
        temp.setFuture("null");
        temp.setAmbiguity(1);

        if (-1 == newTagDbhandle->insert<TagProperty>(temp))
            qWarning() << QString("%1 upgrade failed !").arg(bean->getTagName());
    }

    return true;
}

bool TagDbUpgradeUnit::upgradeFileTag()
{
    // read old table
    const auto &filePropertyBean = deepinDbHandle->query<OldFileProperty>().toBeans();
    if (filePropertyBean.isEmpty())
        return true;

    QVariantMap tagPropertyMap;
    QStringList tags;
    for (auto &bean : filePropertyBean) {
        const QString &path = bean->getFilePath();
        if (path.isEmpty())
            continue;

        tags.clear();
        tags.append(bean->getTag1());
        tags.append(bean->getTag2());
        tags.append(bean->getTag3());

        for (const QString &tag : tags) {
            if (tag.isEmpty())
                continue;

            FileTagInfo info;
            info.setFilePath(bean->getFilePath());
            info.setTagName(tag);
            info.setTagOrder(0);
            info.setFuture("null");

            if (-1 == newTagDbhandle->insert<FileTagInfo>(info))
                qWarning() << QString("%1 upgrade failed !").arg(bean->getFilePath());
        }
    }

    return true;
}

bool TagDbUpgradeUnit::checkOldDatabase()
{
    using namespace dfmio;
    const auto &dbPath1 = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationSharePath).toLocal8Bit(),
                                                  "/database",
                                                  kTagOldDbName1,
                                                  nullptr);

    if (!checkDatabaseFile(dbPath1))
        return false;

    QSqlDatabase db1 { SqliteConnectionPool::instance().openConnection(dbPath1) };
    if (!db1.isValid() || db1.isOpenError())
        return false;

    if (!chechTable(db1, kTagOldDb1TableTagProperty))
        return false;

    mainDbHandle = new SqliteHandle(dbPath1);

    const auto &dbPath2 = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationSharePath).toLocal8Bit(),
                                                  "/database",
                                                  kTagOldDbName2,
                                                  nullptr);

    QSqlDatabase db2 { SqliteConnectionPool::instance().openConnection(dbPath2) };
    if (!db2.isValid() || db2.isOpenError())
        return false;

    if (!chechTable(db2, kTagOldDb2TableFileProperty))
        return false;

    deepinDbHandle = new SqliteHandle(dbPath2);
    return true;
}

bool TagDbUpgradeUnit::checkNewDatabase()
{
    using namespace dfmio;
    const auto &dbPath = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                 "/deepin/dde-file-manager/database",
                                                 kTagNewDbName,
                                                 nullptr);

    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbPath) };
    if (!db.isValid() || db.isOpenError())
        return false;

    if (!chechTable(db, kTagNewTableTagProperty, true))
        return false;

    if (!chechTable(db, kTagNewTableFileTags, true))
        return false;

    newTagDbhandle = new SqliteHandle(dbPath);
    return true;
}
