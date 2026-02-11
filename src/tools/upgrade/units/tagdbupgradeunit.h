// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDBUPGRADEUNIT_H
#define TAGDBUPGRADEUNIT_H

#include "core/upgradeunit.h"
#include <dfm-base/base/db/sqlitehandle.h>

DFMBASE_USE_NAMESPACE

class QSqlDatabase;
namespace dfm_upgrade {
class TagDbUpgradeUnit : public UpgradeUnit
{
public:
    TagDbUpgradeUnit();
    ~TagDbUpgradeUnit();

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    bool upgradeTagDb();

    bool upgradeTagProperty();
    bool upgradeFileTag();

    bool checkOldDatabase();
    bool checkNewDatabase();

    bool chechTable(SqliteHandle *handle, const QString &tableName, bool newTable = false);
    bool createTableForNewDb(const QString &tableName);

    bool upgradeData();
    QString getColorRGB(const QString &color);
    QString checkFileUrl(const QString &filerUrl);

private:
    SqliteHandle *mainDbHandle { nullptr };
    SqliteHandle *deepinDbHandle { nullptr };
    SqliteHandle *newTagDbhandle { nullptr };
};
}

#endif   // TAGDBUPGRADEUNIT_H
