// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTDATAINTERFACE_H
#define REPORTDATAINTERFACE_H

#include <QJsonObject>
#include <QVariantMap>

namespace dfmplugin_utils {

class ReportDataInterface
{
public:
    ReportDataInterface() = default;
    virtual ~ReportDataInterface() = default;
    virtual QString type() const = 0;
    virtual QJsonObject prepareData(const QVariantMap &args) const = 0;
};

}
#endif   // REPORTDATAINTERFACE_H
