// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPROXYMANAGER_H
#define DEVICEPROXYMANAGER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QObject>

#define DevProxyMng DFMBASE_NAMESPACE::DeviceProxyManager::instance()

class OrgDeepinFilemanagerDaemonDeviceManagerInterface;
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
    static DeviceProxyManager *instance();

    const OrgDeepinFilemanagerDaemonDeviceManagerInterface *getDBusIFace() const;

    // device info getter
    QStringList getAllBlockIds(GlobalServerDefines::DeviceQueryOptions opts = GlobalServerDefines::DeviceQueryOption::kNoCondition);
    QStringList getAllBlockIdsByUUID(const QStringList &uuids, GlobalServerDefines::DeviceQueryOptions opts = GlobalServerDefines::DeviceQueryOption::kNoCondition);
    QStringList getAllProtocolIds();
    QVariantMap queryBlockInfo(const QString &id, bool reload = false);
    QVariantMap queryProtocolInfo(const QString &id, bool reload = false);

    // device operation
    void reloadOpticalInfo(const QString &id);

    bool initService();
    bool isDBusRuning();

    bool isFileOfExternalMounts(const QString &filePath);
    bool isFileOfProtocolMounts(const QString &filePath);
    bool isFileOfExternalBlockMounts(const QString &filePath);
    bool isFileFromOptical(const QString &filePath);
    bool isMptOfDevice(const QString &filePath, QString &id);
    QVariantMap queryDeviceInfoByPath(const QString &path, bool reload = false);

    // device signals
Q_SIGNALS:
    void devSizeChanged(const QString &id, qint64 total, qint64 avai);

    void blockDevPropertyChanged(const QString &id, const QString &property, const QVariant &val);
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDevAdded(const QString &id);
    void blockDevRemoved(const QString &id, const QString &oldMpt);
    void blockDevMounted(const QString &id, const QString &mpt);
    void blockDevUnmounted(const QString &id, const QString &oldMpt);
    void blockDevLocked(const QString &id);
    void blockDevUnlocked(const QString &id, const QString &cleartextId);
    void blockDevFsAdded(const QString &id);
    void blockDevFsRemoved(const QString &id);

    void protocolDevAdded(const QString &id);
    void protocolDevRemoved(const QString &id, const QString &oldMpt);
    void protocolDevMounted(const QString &id, const QString &mpt);
    void protocolDevUnmounted(const QString &id, const QString &oldMpt);

    void devMngDBusRegistered();
    void devMngDBusUnregistered();

private:
    explicit DeviceProxyManager(QObject *parent = nullptr);
    virtual ~DeviceProxyManager() override;

private:
    QScopedPointer<DeviceProxyManagerPrivate> d;
};

}   // namespace dfmbase

#endif   // DEVICEPROXYMANAGER_H
