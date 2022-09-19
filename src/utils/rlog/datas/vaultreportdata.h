// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREPORTDATA_H
#define VAULTREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class VaultReportData : public ReportDataInterface
{
public:
    enum VaultMode{
      Created = 1, //创建保险箱
      Deleted      //删除保险箱
    };
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // VAULTREPORTDATA_H
