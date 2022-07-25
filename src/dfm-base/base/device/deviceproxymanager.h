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
#ifndef DEVICEPROXYMANAGER_H
#define DEVICEPROXYMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <QObject>

#define DevProxyMng DFMBASE_NAMESPACE::DeviceProxyManager::instance()

class DeviceManagerInterface;
namespace dfmbase {

class DeviceProxyManagerPrivate;
/*!
 * \brief The DeviceProxyManager class
 * this class is a proxy used to access devices via API or DBus and will determind which
 * to uses by the DBus status.
 * and this is a singleton class.
 */
class DeviceProxyManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceProxyManager)

public:
    inline static DeviceProxyManager *instance()
    {
        static DeviceProxyManager ins;
        return &ins;
    }

    const DeviceManagerInterface *getDBusIFace() const;

    // device info getter
    QStringList getAllBlockIds(GlobalServerDefines::DeviceQueryOptions opts = GlobalServerDefines::DeviceQueryOption::kNoCondition);
    QStringList getAllBlockIdsByUUID(const QStringList &uuids, GlobalServerDefines::DeviceQueryOptions opts = GlobalServerDefines::DeviceQueryOption::kNoCondition);
    QStringList getAllProtocolIds();
    QVariantMap queryBlockInfo(const QString &id, bool reload = false);
    QVariantMap queryProtocolInfo(const QString &id, bool reload = false);

    // device operation
    void detachBlockDevice(const QString &id);
    void detachProtocolDevice(const QString &id);
    void detachAllDevices();
    void reloadOpticalInfo(const QString &id);

    bool connectToService();
    bool isMonitorWorking();
    bool isDBusRuning();

    bool isFileOfExternalMounts(const QString &filePath);
    bool isFileOfProtocolMounts(const QString &filePath);
    bool isFileFromOptical(const QString &filePath);
    bool isMptOfDevice(const QString &filePath, QString &id);

    // device signals
signals:
    void devSizeChanged(const QString &id, qint64 total, qint64 avai);

    void blockDevPropertyChanged(const QString &id, const QString &property, const QVariant &val);
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDevAdded(const QString &id);
    void blockDevRemoved(const QString &id);
    void blockDevMounted(const QString &id, const QString &mpt);
    void blockDevUnmounted(const QString &id);
    void blockDevLocked(const QString &id);
    void blockDevUnlocked(const QString &id, const QString &cleartextId);
    void blockDevFsAdded(const QString &id);
    void blockDevFsRemoved(const QString &id);

    void protocolDevAdded(const QString &id);
    void protocolDevRemoved(const QString &id);
    void protocolDevMounted(const QString &id, const QString &mpt);
    void protocolDevUnmounted(const QString &id);

    void devMngDBusRegistered();
    void devMngDBusUnregistered();

private:
    explicit DeviceProxyManager(QObject *parent = nullptr);
    virtual ~DeviceProxyManager() override;

private:
    QScopedPointer<DeviceProxyManagerPrivate> d;
};

}

#endif   // DEVICEPROXYMANAGER_H
