// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enterdirreportdata.h"

#include <QDateTime>

using namespace dfmplugin_utils;

QString EnterDirReportData::type() const
{
    return "EnterDirectory";
}

QJsonObject EnterDirReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = args;
    data.insert("tid", 1000500007);
    data.insert("sysTime", QDateTime::currentDateTime().toSecsSinceEpoch());
    return QJsonObject::fromVariantMap(data);
}
