// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockmountreportdata.h"

#include <QVariantMap>
#include <QDebug>
#include <QDateTime>

using namespace dfmplugin_utils;

static constexpr int kBlockMountTid { 1000500004 };
static constexpr char kReportType[] { "BlockMount" };

QString BlockMountReportData::type() const
{
    return kReportType;
}

QJsonObject BlockMountReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", kBlockMountTid);
    temArgs.insert("opTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(temArgs);
}
