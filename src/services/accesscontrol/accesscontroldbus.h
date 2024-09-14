// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSCONTROLDBUS_H
#define ACCESSCONTROLDBUS_H

#include <QObject>
#include <QDBusContext>

#include <dfm-mount/base/dmount_global.h>

DFM_MOUNT_BEGIN_NS
class DBlockMonitor;
DFM_MOUNT_END_NS

class AccessControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.AccessControlManager")

public:
    enum ErrCode {
        kNoError = 0,
        kInvalidArgs,
        kInvalidInvoker,
    };

    struct MountArgs
    {
        QString devDesc;
        QString mountPoint;
        QString fileSystem;
    };

    explicit AccessControlDBus(const char *name, QObject *parent = nullptr);
    ~AccessControlDBus();

public slots:
    QString SetAccessPolicy(const QVariantMap &policy);
    QVariantList QueryAccessPolicy();
    QString SetVaultAccessPolicy(const QVariantMap &policy);
    QVariantList QueryVaultAccessPolicy();
    int QueryVaultAccessPolicyVisible();
    QString FileManagerReply(int policystate);
    void ChangeDiskPassword(const QString &oldPwd, const QString &newPwd);
    bool Chmod(const QString &path, uint mode);

private slots:
    void onBlockDevAdded(const QString &deviceId);
    void onBlockDevMounted(const QString &deviceId, const QString &mountPoint);

signals:
    void AccessPolicySetFinished(const QVariantMap &policy);
    void DeviceAccessPolicyChanged(const QVariantList &policy);
    void AccessVaultPolicyNotify();
    void DiskPasswordChecked(int code);
    void DiskPasswordChanged(int code);

private:
    void initConnect();
    void changeMountedOnInit();
    void changeMountedBlock(int mode, const QString &device = "");
    void changeMountedOptical(int mode, const QString &device = "");
    void changeMountedProtocol(int mode, const QString &device = "");
    bool checkAuthentication(const QString &id);

private:
    QMap<int, QPair<QString, int>> globalDevPolicies;
    QMap<QString, int> globalVaultHidePolicies;
    QMap<int, QString> errMsg;
    QScopedPointer<DFMMOUNT::DBlockMonitor> monitor;
};

#endif   // ACCESSCONTROLDBUS_H
