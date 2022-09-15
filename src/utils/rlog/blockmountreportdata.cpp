// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockmountreportdata.h"

#include <QVariantMap>
#include <QDateTime>

static constexpr int BlockMountTid { 1000500004 };

QString BlockMountReportData::type() const
{
    return "BlockMount";
}

QJsonObject BlockMountReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", BlockMountTid);
    temArgs.insert("opTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(temArgs);
}
