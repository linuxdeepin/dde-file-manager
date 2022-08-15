#ifndef SMBREPORTDATA_H
#define SMBREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class SmbReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // SMBREPORTDATA_H
