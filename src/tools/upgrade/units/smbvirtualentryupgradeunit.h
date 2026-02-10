// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBVIRTUALENTRYUPGRADEUNIT_H
#define SMBVIRTUALENTRYUPGRADEUNIT_H

#include "core/upgradeunit.h"
#include "beans/virtualentrydata.h"

#include <dfm-base/base/db/sqlitehandle.h>

#include <QList>

namespace dfm_upgrade {
class SmbVirtualEntryUpgradeUnit : public UpgradeUnit
{
public:
    SmbVirtualEntryUpgradeUnit();
    ~SmbVirtualEntryUpgradeUnit() override;

    virtual QString name() override;
    virtual bool initialize(const QMap<QString, QString> &args) override;
    virtual bool upgrade() override;

private:
    bool createDB();
    bool createTable();

    QList<VirtualEntryData> readOldItems();
    void clearOldItems();
    VirtualEntryData convertFromMap(const QVariantMap &map);
    void saveToDb(const QList<VirtualEntryData> &entries);

private:
    DFMBASE_NAMESPACE::SqliteHandle *handler { nullptr };
};
}

#endif   // SMBVIRTUALENTRYUPGRADEUNIT_H
