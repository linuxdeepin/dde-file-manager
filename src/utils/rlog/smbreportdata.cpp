#include "smbreportdata.h"

#include <QDateTime>

QString SmbReportData::type() const
{
    return "Smb";
}

QJsonObject SmbReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid",1000500001);
    uint time = QDateTime::currentDateTime().toTime_t();
    temArgs.insert("resultTime",time);
    if (temArgs.value("result").toBool()) {
        temArgs.insert("errorId",0);
        temArgs.insert("errorSysMsg","");
        temArgs.insert("errorUiMsg","");
    }
    return QJsonObject::fromVariantMap(temArgs);
}
