#ifndef VAULTREPORTDATA_H
#define VAULTREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class VaultReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // VAULTREPORTDATA_H
