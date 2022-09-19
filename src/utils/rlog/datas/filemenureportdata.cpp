// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemenureportdata.h"

#include <QDateTime>

QString FileMenuReportData::type() const
{
    return "FileMenu";
}

QJsonObject FileMenuReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = args;
    data.insert("tid", 1000500005);
    data.insert("sysTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(data);
}
