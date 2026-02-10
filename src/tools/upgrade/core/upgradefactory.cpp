// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradefactory.h"
#include "units/unitlist.h"

#include <QLoggingCategory>
#include <QElapsedTimer>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)
using namespace dfm_upgrade;

UpgradeFactory::UpgradeFactory()
{
}

void UpgradeFactory::previous(const QMap<QString, QString> &args)
{
    units = createUnits();
    qCInfo(logToolUpgrade) << QString("Successfully loaded %1 units").arg(units.size());
    for (auto unit = units.begin(); unit != units.end();) {
        QString name = (*unit)->name();
        qCInfo(logToolUpgrade) << "Initializing unit:" << name;
        if (!(*unit)->initialize(args)) {
            qCCritical(logToolUpgrade) << "Failed to initialize unit:" << name;
            unit = units.erase(unit);
        } else {
            qCInfo(logToolUpgrade) << "Successfully initialized unit:" << name;
            unit++;
        }
    }
}

void UpgradeFactory::doUpgrade()
{
    for (auto unit = units.begin(); unit != units.end(); unit++) {
        QString name = (*unit)->name();
        qCInfo(logToolUpgrade) << "Starting upgrade for unit:" << name;
        if (!(*unit)->upgrade()) {
            qCCritical(logToolUpgrade) << "Failed to upgrade unit:" << name;
        }
    }
}

void UpgradeFactory::completed()
{
    for (auto unit = units.begin(); unit != units.end(); unit++) {
        QString name = (*unit)->name();
        qCInfo(logToolUpgrade) << "Completing unit:" << name;
        (*unit)->completed();
    }
}
