/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     zhangsheng <zhangsheng@uniontech.com>
 *
 * Maintainer: zhangsheng <zhangsheng@uniontech.com>
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

#ifndef ACCESSCONTROLMANAGER_H
#define ACCESSCONTROLMANAGER_H

#include <QDBusContext>
#include <QObject>

class AccessControlAdaptor;
class DDiskManager;
class DFileSystemWatcher;

class AccessControlManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","com.deepin.filemanager.daemon.AccessControlManager")

    enum ErrCode{
        NoError = 0,
        InvalidArgs,
        InvalidInvoker,
    };

    struct MountArgs{
        QString devDesc;
        QString mountPoint;
        QString fileSystem;
    };

public:
    explicit AccessControlManager(QObject *parent = nullptr);
    ~AccessControlManager();

    void initConnect();

    static QString ObjectPath;
    static QString PolicyKitActionId;

protected:
    bool checkAuthentication();

public slots:
    QString SetAccessPolicy(const QVariantMap &policy);
    QVariantList QueryAccessPolicy();

    QString SetVaultAccessPolicy(const QVariantMap &policy);
    QVariantList QueryVaultAccessPolicy();
    int QueryVaultAccessPolicyVisible();
    QString FileManagerReply(int policystate);

signals:
    void AccessPolicySetFinished(const QVariantMap &policy);
    void DeviceAccessPolicyChanged(const QVariantList &policy);

    void AccessVaultPolicyNotify();

private slots:
    void onFileCreated(const QString &path, const QString &name);
    void chmodMountpoints(const QString &blockDevicePath, const QByteArray &mountPoint);
    void disconnOpticalDev(const QString &drivePath);

    bool isValidPolicy(const QVariantMap &policy, const QString &realInvoker);
    bool isValidInvoker(uint pid, QString &invokerPath);

    void changeMountedPolicy(const QVariantMap &policy);
    void changeMountedBlock(int mode, const QString &device = "");
    void changeMountedOptical(int mode, const QString &device = "");
    void changeMountedProtocol(int mode, const QString &device = "");

    // 在启动系统的时候对已挂载的设备执行一次策略变更（设备的接入先于 daemon 的启动）
    void changeMountedOnInit();

    int accessMode(const QString &mps); // 获取挂载点访问权限

    void savePolicy(const QVariantMap &policy);
    void loadPolicy();
    void decodeConfig();
    void encodeConfig();

    bool isValidVaultPolicy(const QVariantMap &policy);

    void saveVaultPolicy(const QVariantMap &policy);
    void loadVaultPolicy();

private:
    AccessControlAdaptor *m_accessControlAdaptor = nullptr;
    DDiskManager *m_diskMnanager = nullptr;
    DFileSystemWatcher *m_watcher = nullptr;

    QString m_configPath;
    QStringList m_whiteProcess;

    QMap<int, QPair<QString, int>> m_globalPolicies;
    QMap<int, QString> m_errMsg;

    QString m_vaultConfigPath;
    QMap<QString, int> m_VaultHidePolicies;
};

#endif // ACCESSCONTROLMANAGER_H
