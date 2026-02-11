// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerunit.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;

HeaderUnit::HeaderUnit()
    : UpgradeUnit()
{
}

QString HeaderUnit::name()
{
    return "Header";
}

bool HeaderUnit::initialize(const QMap<QString, QString> &args)
{
    qCInfo(logToolUpgrade) << "begin upgrade. the args are" << args;
    time.start();
    return true;
}

bool HeaderUnit::upgrade()
{
    qCInfo(logToolUpgrade) << "init all units spend:" << time.elapsed();
    time.start();
    return true;
}

void HeaderUnit::completed()
{
    qCInfo(logToolUpgrade) << "all units upgraded and spended:" << time.elapsed();
}
