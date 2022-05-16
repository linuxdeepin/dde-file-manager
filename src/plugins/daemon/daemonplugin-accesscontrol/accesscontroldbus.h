/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.AccessControlManager")

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

    explicit AccessControlDBus(QObject *parent = nullptr);
    ~AccessControlDBus();

public slots:
    QString SetAccessPolicy(const QVariantMap &policy);
    QVariantList QueryAccessPolicy();
    QString SetVaultAccessPolicy(const QVariantMap &policy);
    QVariantList QueryVaultAccessPolicy();
    int QueryVaultAccessPolicyVisible();
    QString FileManagerReply(int policystate);

private slots:
    void onBlockDevAdded(const QString &deviceId);
    void onBlockDevMounted(const QString &deviceId, const QString &mountPoint);

signals:
    void AccessPolicySetFinished(const QVariantMap &policy);
    void DeviceAccessPolicyChanged(const QVariantList &policy);
    void AccessVaultPolicyNotify();

private:
    void initConnect();
    void changeMountedOnInit();
    void changeMountedBlock(int mode, const QString &device = "");
    void changeMountedOptical(int mode, const QString &device = "");
    void changeMountedProtocol(int mode, const QString &device = "");

private:
    QMap<int, QPair<QString, int>> globalDevPolicies;
    QMap<QString, int> globalVaultHidePolicies;
    QMap<int, QString> errMsg;
    QScopedPointer<DFMMOUNT::DBlockMonitor> monitor;
};

#endif   // ACCESSCONTROLDBUS_H
