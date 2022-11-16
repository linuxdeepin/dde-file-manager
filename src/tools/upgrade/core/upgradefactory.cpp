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
#include "upgradefactory.h"
#include "units/unitlist.h"

#include <QDebug>

using namespace dfm_upgrade;

UpgradeFactory::UpgradeFactory()
{

}

void UpgradeFactory::previous(const QMap<QString, QString> &args)
{
    units = createUnits();
    qInfo() << QString("load %0 units").arg(units.size());

    for (auto unit = units.begin(); unit != units.end();) {
        QString name = (*unit)->name();
        qInfo() << "initialize unit" << name;
        if (!(*unit)->initialize(args)) {
            qCritical() << "fail to init" << name;
            unit = units.erase(unit);
        } else {
            unit++;
        }
    }
}

void UpgradeFactory::doUpgrade()
{
    for (auto unit = units.begin(); unit != units.end(); unit++) {
        QString name = (*unit)->name();
        qInfo() << "upgrade unit" << name;
        if (!(*unit)->upgrade()) {
            qCritical() << "fail to upgrade" << name;
        }
    }
}

void UpgradeFactory::completed()
{
    for (auto unit = units.begin(); unit != units.end(); unit++) {
        QString name = (*unit)->name();
        qInfo() << "complete unit" << name;
        (*unit)->completed();
    }
}
