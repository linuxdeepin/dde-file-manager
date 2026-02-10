// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPORTLOGEVENTRECEIVER_H
#define REPORTLOGEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <dfm-mount/base/dmount_global.h>

#include <QObject>

namespace dfmplugin_utils {
class ReportLogEventReceiver : public QObject
{
    Q_OBJECT
public:
    explicit ReportLogEventReceiver(QObject *parent = nullptr);

    void bindEvents();

    void commit(const QString &type, const QVariantMap &args);
    void handleMenuData(const QString &name, const QList<QUrl> &urlList);

public Q_SLOTS:
    void handleBlockMountData(const QString &id, bool result);
    void handleMountNetworkResult(const QString &address, bool ret, DFMMOUNT::DeviceError err, const QString &msg);
    void handleDesktopStartupData(const QString &key, const QVariant &data);

private:
    void lazyBindCommitEvent(const QString &plugin, const QString &space);
    void lazyBindMenuDataEvent(const QString &plugin, const QString &space);
};
}
#endif   // REPORTLOGEVENTRECEIVER_H
