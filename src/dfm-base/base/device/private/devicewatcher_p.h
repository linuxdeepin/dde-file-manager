/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DEVICEWATCHER_P_H
#define DEVICEWATCHER_P_H

#include "dfm-base/dfm_base_global.h"

#include <QTimer>
#include <QMutex>
#include <QHash>
#include <qt5/QtCore/qobjectdefs.h>

#include <dfm-mount/base/dfmmount_global.h>

DFMBASE_BEGIN_NAMESPACE

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

DFMBASE_END_NAMESPACE

#endif   // DEVICEWATCHER_P_H
