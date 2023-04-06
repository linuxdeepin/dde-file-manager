// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEWATCHER_P_H
#define DEVICEWATCHER_P_H

#include <QTimer>
#include <QMutex>
#include <QHash>
#include <QtCore/qobjectdefs.h>

#include <dfm-mount/base/dmount_global.h>

namespace dfmbase {

struct DevStorage
{
    quint64 total { 0 };
    quint64 avai { 0 };
    quint64 used { 0 };

    inline bool operator==(const DevStorage &other)
    {
        return total == other.total && avai == other.avai && used == other.used;
    }
    inline bool operator!=(const DevStorage &other)
    {
        return !(this->operator==(other));
    }
    inline bool isValid()
    {
        return this->operator!=({});
    }
};

class DeviceWatcher;
class DeviceWatcherPrivate : public QObject
{
    Q_OBJECT
    friend class DeviceWatcher;

public:
    explicit DeviceWatcherPrivate(DeviceWatcher *qq);

private Q_SLOTS:
    void queryUsageAsync();
    void updateStorage(const QString &id, quint64 total, quint64 avai);

private:
    void queryUsageOfItem(const QVariantMap &itemData, DFMMOUNT::DeviceType type);
    DevStorage queryUsageOfBlock(const QVariantMap &itemData);
    DevStorage queryUsageOfProtocol(const QVariantMap &itemData);

private:
    DeviceWatcher *q { nullptr };

    QTimer pollingTimer;
    const int kPollingInterval = 10000;

    QHash<QString, QVariantMap> allBlockInfos;
    QHash<QString, QVariantMap> allProtocolInfos;

    QList<QMetaObject::Connection> connections;
    bool isWatching { false };
};

}

#endif   // DEVICEWATCHER_P_H
