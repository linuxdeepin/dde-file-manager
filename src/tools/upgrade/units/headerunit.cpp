// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerunit.h"

#include <QDebug>

using namespace dfm_upgrade;

HeaderUnit::HeaderUnit() : UpgradeUnit()
{

}

QString HeaderUnit::name()
{
    return "Header";
}

bool HeaderUnit::initialize(const QMap<QString, QString> &args)
{
    qInfo() << "begin upgrade. the args are" << args;
    time.start();
    return true;
}

bool HeaderUnit::upgrade()
{
    qInfo() << "init all units spend:" << time.elapsed();
    time.start();
    return true;
}

void HeaderUnit::completed()
{
    qInfo() << "all units upgraded and spended:" << time.elapsed();
}
