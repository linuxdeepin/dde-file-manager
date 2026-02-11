// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTLOGMANAGER_H
#define REPORTLOGMANAGER_H

#include "dfmplugin_utils_global.h"

#include <dfm-base/base/device/devicemanager.h>

#include <QObject>

namespace dfmplugin_utils {
class ReportLogWorker;
class ReportLogManager : public QObject
{
    Q_OBJECT
public:
    static ReportLogManager *instance();
    void init();

    void commit(const QString &type, const QVariantMap &args);
    void reportMenuData(const QString &name, const QList<QUrl> &urlList);
    void reportNetworkMountData(bool ret, DFMMOUNT::DeviceError err, const QString &msg);
    void reportBlockMountData(const QString &id, bool result);
    void reportDesktopStartUp(const QString &key, const QVariant &data);

Q_SIGNALS:
    void requestCommitLog(const QString &type, const QVariantMap &args);
    void requestReportMenuData(const QString &name, const QList<QUrl> &urlList);
    void requestReportNetworkMountData(bool ret, DFMMOUNT::DeviceError err, const QString &msg);
    void requestReportBlockMountData(const QString &id, bool result);
    void requestReportDesktopStartUp(const QString &key, const QVariant &data);

private:
    explicit ReportLogManager(QObject *parent = nullptr);
    ~ReportLogManager();

    void initConnection();

    QThread *reportWorkThread { nullptr };
    ReportLogWorker *reportWorker { nullptr };
};
}
#endif // REPORTLOGMANAGER_H
