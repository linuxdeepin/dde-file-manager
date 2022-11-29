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
#ifndef UNITLIST_H
#define UNITLIST_H

#include "core/upgradeunit.h"

// units
#include "headerunit.h"
#include "dconfigupgradeunit.h".h "

// units end

#include <QList>
#include <QSharedPointer>

#define RegUnit(unit) \
    QSharedPointer<UpgradeUnit>(new unit)

namespace dfm_upgrade {

inline QList<QSharedPointer<UpgradeUnit>> createUnits()
{
    return QList<QSharedPointer<UpgradeUnit>> {
        RegUnit(dfm_upgrade::HeaderUnit),
        RegUnit(dfm_upgrade::DConfigUpgradeUnit)
    };
}

}

#endif   // UNITLIST_H
