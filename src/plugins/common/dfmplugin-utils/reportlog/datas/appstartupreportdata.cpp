// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appstartupreportdata.h"

#include <QDateTime>

using namespace dfmplugin_utils;

QString AppStartupReportData::type() const
{
    return "AppStartup";
}

QJsonObject AppStartupReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap data = args;
    data.insert("tid", 1000500006);
    data.insert("sysTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(data);
}
