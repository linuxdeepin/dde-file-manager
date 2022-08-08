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
#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

#include <dfm-mount/base/dmount_global.h>

namespace dfmbase {

class DeviceManager;
class DeviceWatcherPrivate;
/*!
 * \brief The DeviceWatcher class
 * this class is designed to watch the changes of devices, and update the cached info of device manager.
 */
class DeviceWatcher : public QObject
{
    Q_OBJECT
    friend class DeviceManager;

public:
    explicit DeviceWatcher(QObject *parent = nullptr);
    virtual ~DeviceWatcher() override;

    QVariantMap getDevInfo(const QString &id, DFMMOUNT::DeviceType type, bool reload);
    QStringList getDevIds(DFMMOUNT::DeviceType type);
    QStringList getSiblings(const QString &id);

    void startPollingUsage();
    void stopPollingUsage();

    void startWatch();
    void stopWatch();
    void initDevDatas();

    void queryOpticalDevUsage(const QString &id);

private Q_SLOTS:
    void onBlkDevAdded(const QString &id);
    void onBlkDevRemoved(const QString &id);
    void onBlkDevMounted(const QString &id, const QString &mpt);
    void onBlkDevUnmounted(const QString &id);
    void onBlkDevLocked(const QString &id);
    void onBlkDevUnlocked(const QString &id, const QString &cleartextId);

    void onBlkDevFsAdded(const QString &id);
    void onBlkDevFsRemoved(const QString &id);

    void onBlkDevPropertiesChanged(const QString &id, const QMap<DFMMOUNT::Property, QVariant> &changes);

    void onProtoDevAdded(const QString &id);
    void onProtoDevRemoved(const QString &id);
    void onProtoDevMounted(const QString &id, const QString &mpt);
    void onProtoDevUnmounted(const QString &id);

private:
    QScopedPointer<DeviceWatcherPrivate> d;
};

}

#endif   // DEVICEWATCHER_H
