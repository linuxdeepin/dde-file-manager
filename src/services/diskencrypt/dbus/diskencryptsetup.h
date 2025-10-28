// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTSETUP_H
#define DISKENCRYPTSETUP_H

#include <QObject>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>

#include <qdbusservice.h>

// see https://github.com/linuxdeepin/deepin-service-manager/blob/master/develop-guide.md for more information

class DiskEncryptSetupPrivate;
class DiskEncryptSetup : public QDBusService, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.DiskEncrypt")

public:
    explicit DiskEncryptSetup(QObject *parent = nullptr);

public Q_SLOTS:
    bool InitEncryption(const QVariantMap &args);
    bool ResumeEncryption(const QVariantMap &args);
    bool Decryption(const QDBusUnixFileDescriptor &credentialsFd);
    bool ChangePassphrase(const QDBusUnixFileDescriptor &credentialsFd);
    void SetupAuthArgs(const QDBusUnixFileDescriptor &credentialsFd);
    void IgnoreAuthSetup();

    QString TpmToken(const QString &dev);
    int DeviceStatus(const QString &dev);
    QString HolderDevice(const QString &dev);

    bool IsTaskEmpty();
    bool IsTaskRunning();
    QString PendingDecryptionDevice();

Q_SIGNALS:
    void EncryptProgress(const QString &dev, const QString &devName, double progress);
    void DecryptProgress(const QString &dev, const QString &devName, double progress);

    void InitEncResult(const QVariantMap &result);
    void EncryptResult(const QVariantMap &result);
    void DecryptResult(const QVariantMap &result);
    void ChangePassResult(const QVariantMap &result);

    void WaitAuthInput(const QVariantMap &devInfo);

private:
    DiskEncryptSetupPrivate *m_dptr { nullptr };
};

#endif   // DISKENCRYPTSETUP_H
