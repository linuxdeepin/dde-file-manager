#ifndef SMBREPORTDATA_H
#define SMBREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class SmbReportData : public ReportDataInterface
{
public:
    enum ErrId{
        NoError = 0,  //没有错误
        //Smb error id
        Fetch_Error,  //文件获取错误
        NotMount,     //指定位置未挂载
        Mount_Error   //挂载错误
    };
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // SMBREPORTDATA_H
