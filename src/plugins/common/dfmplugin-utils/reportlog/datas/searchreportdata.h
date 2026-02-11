// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHREPORTDATA_H
#define SEARCHREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

namespace dfmplugin_utils {

class SearchReportData : public ReportDataInterface
{
public:
    enum SearchMode {
        kTurnOn = 1,   //开启全文搜索
        kTurnOff   //关闭全文搜索
    };
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

}
#endif   // SEARCHREPORTDATA_H
