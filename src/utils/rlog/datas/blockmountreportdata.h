// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCKMOUNTREPORTDATA_H
#define BLOCKMOUNTREPORTDATA_H

#include "reportdatainterface.h"

class BlockMountReportData: public ReportDataInterface
{
    // ReportDataInterface interface
public:
    virtual QString type() const override;
    virtual QJsonObject prepareData(const QVariantMap &args) const override;

    static void report(const QVariantMap &datas);
};

#endif // BLOCKMOUNTREPORTDATA_H
