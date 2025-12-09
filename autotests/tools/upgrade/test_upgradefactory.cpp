// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/upgradefactory.h"
#include "core/upgradeunit.h"

#include "units/unitlist.h"

#include "builtininterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace dfm_upgrade;
class TestUpgradeUnit : public UpgradeUnit
{
public:
    QString name() {return "test";}
    bool initialize(const QMap<QString, QString> &args){
        f1 = true;
        return !args.isEmpty();
    }
    bool upgrade(){ f2 = true; return true;}
    void completed(){f3 = true;}
    bool f1 = false;
    bool f2 = false;
    bool f3 = false;
};

TEST(UpgradeFactory, previous)
{
    stub_ext::StubExt stub;
    auto unit = new TestUpgradeUnit;
    QSharedPointer<UpgradeUnit> pUnit(unit);
    stub.set_lamda(&createUnits, [pUnit](){
        return QList<QSharedPointer<UpgradeUnit>> {
            pUnit
        };
    });

    UpgradeFactory fac;
    fac.previous({});
    EXPECT_TRUE(fac.units.isEmpty());
    EXPECT_TRUE(unit->f1);
    EXPECT_FALSE(unit->f2);
    EXPECT_FALSE(unit->f3);

    unit->f1 = false;
    unit->f2 = false;
    unit->f3 = false;

    fac.previous({{"test","0"}});
    EXPECT_EQ(1, fac.units.size());
    EXPECT_TRUE(unit->f1);
    EXPECT_FALSE(unit->f2);
    EXPECT_FALSE(unit->f3);
}

TEST(UpgradeFactory, doUpgrade)
{
    stub_ext::StubExt stub;
    auto unit = new TestUpgradeUnit;
    QSharedPointer<UpgradeUnit> pUnit(unit);
    UpgradeFactory fac;
    fac.units = QList<QSharedPointer<UpgradeUnit>> { pUnit };

    fac.doUpgrade();
    EXPECT_TRUE(unit->f2);
}

TEST(UpgradeFactory, completed)
{
    stub_ext::StubExt stub;
    auto unit = new TestUpgradeUnit;
    QSharedPointer<UpgradeUnit> pUnit(unit);

    UpgradeFactory fac;
    fac.units = QList<QSharedPointer<UpgradeUnit>> { pUnit };

    fac.completed();
    EXPECT_TRUE(unit->f3);
}
