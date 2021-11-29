/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    void initializeConnect();
    Q_SLOT void onSizeChanged(const QString &id, qint64 total, qint64 free);

private:
    QVariantMap data;
    const QString ddeI18nSym { QStringLiteral("_dde_") };
};

#endif   // DATTACHEDBLOCKDEVICE_H
