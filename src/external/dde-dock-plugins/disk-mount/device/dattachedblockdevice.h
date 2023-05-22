// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATTACHEDBLOCKDEVICE_H
#define DATTACHEDBLOCKDEVICE_H

#include "dattacheddevice.h"

#include <QUrl>
#include <QPointer>
#include <QVariantMap>

class DAttachedBlockDevice final : public QObject, public DAttachedDevice
{
public:
    explicit DAttachedBlockDevice(const QString &id, QObject *parent = nullptr);
    virtual ~DAttachedBlockDevice() override;
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

private:
    const QString ddeI18nSym { QStringLiteral("_dde_") };
    QVariantMap info;
};

#endif   // DATTACHEDBLOCKDEVICE_H
