// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBREPORTDATA_H
#define SMBREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

class SmbReportData : public ReportDataInterface
{
public:
    enum ErrId{
        NoError = 0,  //没有错误
        Fetch_Error,  //文件获取错误
        NotMount,     //指定位置未挂载
        Mount_Error,   //挂载错误
        ShareFoldList_Error,   //共享目录列表获取错误(已挂载过的ip地址，后来遇远端断网)
        UserCancel_Error       //用户主动取消挂载操作（密码对话框被取消）
    };
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif // SMBREPORTDATA_H
