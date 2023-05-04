// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATTACHEDPROTOCOLDEVICE_H
#define DATTACHEDPROTOCOLDEVICE_H

#include "dattacheddevice.h"

#include <QUrl>
#include <QPointer>
#include <QVariantMap>

namespace dfmmount {
class DProtocolDevice;
}

class DAttachedProtocolDevice final : public QObject, public DAttachedDevice
{
    Q_OBJECT
public:
    explicit DAttachedProtocolDevice(const QString &id, QObject *parent = nullptr);
    virtual ~DAttachedProtocolDevice() override;
    bool isValid() override;
    void detach() override;
    bool detachable() override;
    QString displayName() override;
    bool deviceUsageValid() override;
    QPair<quint64, quint64> deviceUsage() override;
    QString iconName() override;
    QUrl mountpointUrl() override;
    QUrl accessPointUrl() override;

protected:
    void query() override;
    bool checkNetwork();
    bool parseHostAndPort(QString &host, QString &port);

private:
    QSharedPointer<dfmmount::DProtocolDevice> device;
    quint64 lastNetCheck = 0;
    quint64 latestTotalSize = 0;
    quint64 latestFreeSize = 0;
    bool isNetworkDev = false;
};

#endif   // DATTACHEDPROTOCOLDEVICE_H
