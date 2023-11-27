// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <dfm-base/dfm_base_global.h>

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
    void updateOpticalDevUsage(const QString &id, const QString &mpt);
    void saveOpticalDevUsage(const QString &id, const QVariantMap &data);

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
