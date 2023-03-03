// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEWATCHER_P_H
#define DEVICEWATCHER_P_H

#include "dfm-base/dfm_base_global.h"

#include <QTimer>
#include <QMutex>
#include <QHash>
#include <qt5/QtCore/qobjectdefs.h>

#include <dfm-mount/base/dmount_global.h>

namespace dfmbase {

class DeviceWatcher;
class DeviceWatcherPrivate
{
    friend class DeviceWatcher;

public:
    explicit DeviceWatcherPrivate(DeviceWatcher *qq);

private:
    void queryUsage();
    void queryUsage(DFMMOUNT::DeviceType type, const QHash<QString, QVariantMap> &datas);
    void queryUsage(const QString &id, const QString &mpt, DFMMOUNT::DeviceType type, bool notifyIfChanged);

private:
    DeviceWatcher *q { nullptr };

    QTimer pollingTimer;
    const int kPollingInterval = 10000;

    QMutex blkMtx;
    QMutex protoMtx;
    QHash<QString, QVariantMap> allBlockInfos;
    QHash<QString, QVariantMap> allProtocolInfos;

    QList<QMetaObject::Connection> connections;
    bool isWatching { false };
};

}

#endif   // DEVICEWATCHER_P_H
