#include "vaultreportdata.h"

#include <QDateTime>

QString VaultReportData::type() const
{
    return "Vault";
}

QJsonObject VaultReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", 1000500000);
    temArgs.insert("sysTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(temArgs);
}
