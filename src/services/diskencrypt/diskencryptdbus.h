// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTDBUS_H
#define DISKENCRYPTDBUS_H

#include <QObject>
#include <qdbusservice.h>
#include <QDBusContext>
#include <QDBusServiceWatcher>

class DiskEncryptDBus : public QDBusService, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.DiskEncrypt")
public:
    explicit DiskEncryptDBus(QObject *parent = nullptr);
    ~DiskEncryptDBus();

public Q_SLOTS:
    QString PrepareEncryptDisk(const QVariantMap &params);
    QString DecryptDisk(const QVariantMap &params);
    QString ChangeEncryptPassphress(const QVariantMap &params);
    void IgnoreParamRequest();
    void ResumeEncryption(const QString &device);
    void SetEncryptParams(const QVariantMap &params);
    QString QueryTPMToken(const QString &device);
    int EncryptStatus(const QString &device);
    bool HasPendingTask();
    bool IsWorkerRunning();
    QString UnfinishedDecryptJob();

Q_SIGNALS:
    void PrepareEncryptDiskResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void EncryptDiskResult(const QString &device, const QString &devName, int errCode, const QString &msg);
    void DecryptDiskResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void ChangePassphressResult(const QString &device, const QString &devName, const QString &jobID, int errCode);
    void EncryptProgress(const QString &device, const QString &devName, double progress);
    void DecryptProgress(const QString &device, const QString &devName, double progress);
    void RequestEncryptParams(const QVariantMap &encConfig);

private Q_SLOTS:
    void onFstabDiskEncProgressUpdated(const QString &dev, qint64 offset, qint64 total);
    void onFstabDiskEncFinished(const QString &dev, int result, const QString &errstr);

private:
    bool checkAuth(const QString &actID);
    void diskCheck();
    bool triggerReencrypt(const QString &device = QString());
    void createReencryptDesktop();
    static void getDeviceMapper(QMap<QString, QString> *, QMap<QString, QString> *, QMap<QString, QString> *);
    static bool updateCrypttab();
    static int isEncrypted(const QString &target, const QString &source);

private:
    QString currentEncryptingDevice;
    QString deviceName;
    bool running { false };
};

#endif   // DISKENCRYPTDBUS_H
