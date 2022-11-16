/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    QString name() {return "";}
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
    stub.set_lamda(&createUnits, [pUnit](){
        return QList<QSharedPointer<UpgradeUnit>> {
            pUnit
        };
    });

    UpgradeFactory fac;
    fac.previous({{"test","0"}});

    fac.doUpgrade();
    EXPECT_TRUE(unit->f2);
}

TEST(UpgradeFactory, completed)
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
    fac.previous({{"test","0"}});

    fac.completed();
    EXPECT_TRUE(unit->f3);
}
