#include "searchreportdata.h"

QString SearchReportData::type() const
{
    return "Search";
}

QJsonObject SearchReportData::prepareData(const QVariantMap &args) const
{
    //TODO,tid=1000500002
    return QJsonObject::fromVariantMap(args);
}
