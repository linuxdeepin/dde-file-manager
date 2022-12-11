// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TAGDBUPGRADEUNIT_H
#define TAGDBUPGRADEUNIT_H

#include "core/upgradeunit.h"
#include "dfm-base/base/db/sqlitehandle.h"

DFMBASE_USE_NAMESPACE

class QSqlDatabase;
namespace dfm_upgrade {
class TagDbUpgradeUnit : public UpgradeUnit
{
public:
    TagDbUpgradeUnit();

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    bool upgradeTagDb();

    bool upgradeTagProperty();
    bool upgradeFileTag();

    bool checkOldDatabase();
    bool checkNewDatabase();

    bool checkDatabaseFile(const QString &dbPath);
    bool chechTable(const QSqlDatabase &db, const QString &tableName, bool newTable = false);
    bool createTableForNewDb(const QSqlDatabase &db, const QString &tableName);

    bool upgradeData();

private:
    SqliteHandle *mainDbHandle { nullptr };
    SqliteHandle *deepinDbHandle { nullptr };
    SqliteHandle *newTagDbhandle { nullptr };
    QMap<QString, QString> tableInfos;
};
}

#endif   // TAGDBUPGRADEUNIT_H
