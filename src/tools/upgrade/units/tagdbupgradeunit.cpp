// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdbupgradeunit.h"
#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"
#include "beans/oldfileproperty.h"
#include "beans/oldtagproperty.h"
#include "beans/sqlitemaster.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>

#include <dfm-io/dfmio_utils.h>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

static constexpr char kTagOldDbName1[] = ".__main.db";
static constexpr char kTagOldDb1TableTagProperty[] = "tag_property";

static constexpr char kTagOldDbName2[] = ".__deepin.db";
static constexpr char kTagOldDb2TableFileProperty[] = "file_property";
static constexpr char kTagOldDb2TableTagWithFile[] = "tag_with_file";

static constexpr char kTagNewDbName[] = "dfmruntime.db";
static constexpr char kTagNewTableFileTags[] = "file_tags";
static constexpr char kTagNewTableTagProperty[] = "tag_property";

using namespace dfm_upgrade;
DFMBASE_USE_NAMESPACE

TagDbUpgradeUnit::TagDbUpgradeUnit()
{
}

TagDbUpgradeUnit::~TagDbUpgradeUnit()
{
    if (mainDbHandle) {
        delete mainDbHandle;
        mainDbHandle = nullptr;
    }
    if (deepinDbHandle) {
        delete deepinDbHandle;
        deepinDbHandle = nullptr;
    }
    if (newTagDbhandle) {
        delete newTagDbhandle;
        newTagDbhandle = nullptr;
    }
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
    qCInfo(logToolUpgrade) << "Starting tag database upgrade process";

    // check db
    // if checkNewDatabase return false, it's mean that we unable to upgrade
    if (!checkNewDatabase()) {
        qCCritical(logToolUpgrade) << "Failed to check or create new tag database";
        return false;
    }

    // if checkOldDatabase return false mean that we don't need upgrade
    if (!checkOldDatabase()) {
        qCInfo(logToolUpgrade) << "No old tag databases found or no upgrade needed";
        return false;
    }

    return upgradeTagDb();
}

bool TagDbUpgradeUnit::upgradeTagDb()
{
    // upgrade tag_property
    if (!upgradeTagProperty()) {
        qCCritical(logToolUpgrade) << "Failed to upgrade tag properties";
        return false;
    }

    // upgrade file_tags
    if (!upgradeFileTag()) {
        qCCritical(logToolUpgrade) << "Failed to upgrade file tags";
        return false;
    }

    return true;
}

bool TagDbUpgradeUnit::chechTable(SqliteHandle *handle, const QString &tableName, bool newTable)
{
    // check table
    const auto &field = Expression::Field<SqliteMaster>;
    const auto &beanList = handle->query<SqliteMaster>().where(field("type") == "table" && field("name") == tableName).toBeans();

    if (0 == beanList.size())
        return newTable ? createTableForNewDb(tableName) : false;
    return true;
}

bool TagDbUpgradeUnit::createTableForNewDb(const QString &tableName)
{
    bool ret = false;
    if (SqliteHelper::tableName<FileTagInfo>() == tableName) {

        ret = newTagDbhandle->createTable<FileTagInfo>(
                SqliteConstraint::primary("fileIndex"),
                SqliteConstraint::autoIncreament("fileIndex"),
                SqliteConstraint::unique("fileIndex"));
    }

    if (SqliteHelper::tableName<TagProperty>() == tableName) {

        ret = newTagDbhandle->createTable<TagProperty>(
                SqliteConstraint::primary("tagIndex"),
                SqliteConstraint::autoIncreament("tagIndex"),
                SqliteConstraint::unique("tagIndex"));
    }

    return ret;
}

bool TagDbUpgradeUnit::upgradeData()
{
    // upgrade db
    if (!upgradeTagProperty()) {
        qCCritical(logToolUpgrade) << "Failed to upgrade tag properties in upgradeData";
        return false;
    }

    if (!upgradeFileTag()) {
        qCCritical(logToolUpgrade) << "Failed to upgrade file tags in upgradeData";
        return false;
    }

    return true;
}

bool TagDbUpgradeUnit::upgradeTagProperty()
{
    // read old table
    const auto &tagPropertyBean = mainDbHandle->query<OldTagProperty>().toBeans();
    if (tagPropertyBean.isEmpty()) {
        qCInfo(logToolUpgrade) << "No old tag properties found to migrate";
        return true;
    }

    for (auto &bean : tagPropertyBean) {
        TagProperty temp;
        temp.setTagName(bean->getTagName());
        temp.setTagColor(getColorRGB(bean->getTagColor()));
        temp.setFuture("null");
        temp.setAmbiguity(1);

        if (-1 == newTagDbhandle->insert<TagProperty>(temp))
            qCWarning(logToolUpgrade) << QString("%1 upgrade failed !").arg(bean->getTagName());
    }

    return true;
}

QString TagDbUpgradeUnit::getColorRGB(const QString &color)
{
    QMap<QString, QString> colorMap;
    colorMap["Orange"] = "#ffa503";
    colorMap["Red"] = "#ff1c49";
    colorMap["Purple"] = "#9023fc";
    colorMap["Navy-blue"] = "#3468ff";
    colorMap["Azure"] = "#00b5ff";
    colorMap["Grass-green"] = "#58df0a";
    colorMap["Yellow"] = "#fef144";
    colorMap["Gray"] = "#cccccc";

    return colorMap[color];
}

bool TagDbUpgradeUnit::upgradeFileTag()
{
    // read old table
    const auto &filePropertyBean = deepinDbHandle->query<OldFileProperty>().toBeans();
    if (filePropertyBean.isEmpty()) {
        qCInfo(logToolUpgrade) << "No old file properties found to migrate";
        return true;
    }

    for (auto &bean : filePropertyBean) {
        QString curpath = checkFileUrl(bean->getFilePath());
        if (curpath.isEmpty())
            continue;

        FileTagInfo info;
        info.setFilePath(curpath);
        info.setTagName(bean->getTag());
        info.setTagOrder(0);
        info.setFuture("null");

        if (-1 == newTagDbhandle->insert<FileTagInfo>(info))
            qCWarning(logToolUpgrade) << QString("%1 upgrade failed !").arg(bean->getFilePath());
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

    QSqlDatabase db1 { SqliteConnectionPool::instance().openConnection(dbPath1) };
    if (!db1.isValid() || db1.isOpenError()) {
        qCDebug(logToolUpgrade) << "Main database not accessible:" << dbPath1;
        return false;
    }
    db1.close();

    // check ".__main.db" database table
    mainDbHandle = new SqliteHandle(dbPath1);
    if (!chechTable(mainDbHandle, kTagOldDb1TableTagProperty)) {
        qCWarning(logToolUpgrade) << "Main database table validation failed:" << kTagOldDb1TableTagProperty;
        return false;
    }

    const auto &dbPath2 = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationSharePath).toLocal8Bit(),
                                                  "/database",
                                                  kTagOldDbName2,
                                                  nullptr);

    QSqlDatabase db2 { SqliteConnectionPool::instance().openConnection(dbPath2) };
    if (!db2.isValid() || db2.isOpenError()) {
        qCDebug(logToolUpgrade) << "Deepin database not accessible:" << dbPath2;
        return false;
    }
    db2.close();

    // check ".__deepin.db" database table
    deepinDbHandle = new SqliteHandle(dbPath2);
    if (!chechTable(deepinDbHandle, kTagOldDb2TableFileProperty)) {
        qCWarning(logToolUpgrade) << "Deepin database table validation failed:" << kTagOldDb2TableFileProperty;
        return false;
    }

    return true;
}

bool TagDbUpgradeUnit::checkNewDatabase()
{
    using namespace dfmio;
    const auto &dbPath = DFMUtils::buildFilePath(StandardPaths::location(StandardPaths::kApplicationConfigPath).toLocal8Bit(),
                                                 "/deepin/dde-file-manager/database",
                                                 nullptr);

    QDir dir(dbPath);
    if (!dir.exists()) {
        qCDebug(logToolUpgrade) << "Database directory does not exist, creating:" << dbPath;
        dir.mkpath(dbPath);
    }

    const auto &dbFilePath = DFMUtils::buildFilePath(dbPath.toLocal8Bit(),
                                                     kTagNewDbName,
                                                     nullptr);

    QSqlDatabase db { SqliteConnectionPool::instance().openConnection(dbFilePath) };
    if (!db.isValid() || db.isOpenError()) {
        qCCritical(logToolUpgrade) << "Failed to create or open new tag database:" << dbFilePath;
        return false;
    }
    db.close();

    // check ".__tag.db" database table
    newTagDbhandle = new SqliteHandle(dbFilePath);
    if (!chechTable(newTagDbhandle, kTagNewTableTagProperty, true)) {
        qCCritical(logToolUpgrade) << "Failed to validate or create tag property table";
        return false;
    }

    if (!chechTable(newTagDbhandle, kTagNewTableFileTags, true)) {
        qCCritical(logToolUpgrade) << "Failed to validate or create file tags table";
        return false;
    }

    return true;
}

QString TagDbUpgradeUnit::checkFileUrl(const QString &filerUrl)
{
    QStringList paths = filerUrl.split("/");
    if (paths.count() < 3) {
        qCDebug(logToolUpgrade) << "Invalid file URL format, insufficient path components:" << filerUrl;
        return QString();
    }
    paths.removeFirst();
    paths.removeFirst();
    QString curpath = QDir::homePath();
    for (QString p : paths)
        curpath += "/" + p;
    QFileInfo info(curpath);
    if (info.exists()) {
        qCDebug(logToolUpgrade) << "File exists, path validated:" << curpath;
        return curpath;
    } else {
        qCDebug(logToolUpgrade) << "File does not exist, path invalid:" << curpath;
        return QString();
    }
}
