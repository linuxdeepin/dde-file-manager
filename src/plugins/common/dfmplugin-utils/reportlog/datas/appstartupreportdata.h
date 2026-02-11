// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPSTARTUPREPORTDATA_H
#define APPSTARTUPREPORTDATA_H

#include "reportdatainterface.h"

#include <QObject>

namespace dfmplugin_utils {

class AppStartupReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

}
#endif   // APPSTARTUPREPORTDATA_H
