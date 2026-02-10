// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBREPORTDATA_H
#define SMBREPORTDATA_H

#include <QObject>

#include "reportdatainterface.h"

namespace dfmplugin_utils {

class SmbReportData : public ReportDataInterface
{
public:
    enum ErrId {
        kNoError = 0,   //没有错误
        kFetchError,   //文件获取错误
        kNotMount,   //指定位置未挂载
        kMountError,   //挂载错误
        kShareFoldListError,   //共享目录列表获取错误(已挂载过的ip地址，后来遇远端断网)
        kUserCancelError   //用户主动取消挂载操作（密码对话框被取消）
    };
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

}

#endif   // SMBREPORTDATA_H
