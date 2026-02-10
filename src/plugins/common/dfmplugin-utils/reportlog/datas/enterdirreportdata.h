// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTERDIRREPORTDATA_H
#define ENTERDIRREPORTDATA_H

#include "reportdatainterface.h"

#include <QObject>

namespace dfmplugin_utils {

class EnterDirReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

}
#endif // ENTERDIRREPORTDATA_H
