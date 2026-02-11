// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbreportdata.h"

#include <QDateTime>

using namespace dfmplugin_utils;

QString SmbReportData::type() const
{
    return "Smb";
}

QJsonObject SmbReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", 1000500001);
    uint time = QDateTime::currentDateTime().toSecsSinceEpoch();
    temArgs.insert("resultTime", time);
    if (temArgs.value("result").toBool()) {
        temArgs.insert("errorId", 0);
        temArgs.insert("errorSysMsg", "");
        temArgs.insert("errorUiMsg", "");
    }
    return QJsonObject::fromVariantMap(temArgs);
}
