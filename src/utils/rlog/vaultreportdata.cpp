#include "vaultreportdata.h"

QString VaultReportData::type() const
{
    return "Vault";
}

QJsonObject VaultReportData::prepareData(const QVariantMap &args) const
{
    //TODO，id=1000500000
    return QJsonObject::fromVariantMap(args);
}
