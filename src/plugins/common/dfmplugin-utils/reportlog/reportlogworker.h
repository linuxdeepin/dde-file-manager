// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTLOGWORKER_H
#define REPORTLOGWORKER_H

#include "dfmplugin_utils_global.h"
#include "datas/reportdatainterface.h"

#include <dfm-base/base/device/devicemanager.h>

#include <QLibrary>
#include <QJsonObject>

namespace dfmplugin_utils {

class ReportLogWorker : public QObject
{
    Q_OBJECT
public:
    using InitEventLog = bool (*)(const std::string &, bool);
    using WriteEventLog = void (*)(const std::string &);

    explicit ReportLogWorker(QObject *parent = nullptr);
    ~ReportLogWorker();

    bool init();

public Q_SLOTS:
    void commitLog(const QString &type, const QVariantMap &args);
    void handleMenuData(const QString &name, const QList<QUrl> &urlList);
    void handleBlockMountData(const QString &id, bool result);
    void handleDesktopStartUpData(const QString &key, const QVariant &data);
    void handleMountNetworkResult(bool ret, DFMMOUNT::DeviceError err, const QString &msg);

private:
    bool registerLogData(const QString &type, ReportDataInterface *dataObj);
    void commit(const QVariant &args);

    QLibrary logLibrary;
    InitEventLog initEventLogFunc = nullptr;
    WriteEventLog writeEventLogFunc = nullptr;

    QJsonObject commonData;
    QHash<QString, ReportDataInterface *> logDataObj;
};
}

#endif // REPORTLOGWORKER_H
