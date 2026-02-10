// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultreportdata.h"

#include <QDateTime>

using namespace dfmplugin_utils;

QString VaultReportData::type() const
{
    return "Vault";
}

QJsonObject VaultReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", 1000500000);
    temArgs.insert("sysTime", QDateTime::currentDateTime().toSecsSinceEpoch());
    return QJsonObject::fromVariantMap(temArgs);
}
