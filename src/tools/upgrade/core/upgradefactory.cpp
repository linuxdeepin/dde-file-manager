// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradefactory.h"
#include "units/unitlist.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)
using namespace dfm_upgrade;

UpgradeFactory::UpgradeFactory()
{
}

void UpgradeFactory::previous(const QMap<QString, QString> &args)
{
    units = createUnits();
    qCInfo(logToolUpgrade) << QString("load %0 units").arg(units.size());

    for (auto unit = units.begin(); unit != units.end();) {
        QString name = (*unit)->name();
        qCInfo(logToolUpgrade) << "initialize unit" << name;
        if (!(*unit)->initialize(args)) {
            qCCritical(logToolUpgrade) << "fail to init" << name;
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
        qCInfo(logToolUpgrade) << "upgrade unit" << name;
        if (!(*unit)->upgrade()) {
            qCCritical(logToolUpgrade) << "fail to upgrade" << name;
        }
    }
}

void UpgradeFactory::completed()
{
    for (auto unit = units.begin(); unit != units.end(); unit++) {
        QString name = (*unit)->name();
        qCInfo(logToolUpgrade) << "complete unit" << name;
        (*unit)->completed();
    }
}
