#ifndef SEARCHREPORTDATA_H
#define SEARCHREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class SearchReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // SEARCHREPORTDATA_H
