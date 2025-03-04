// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTSETUP_P_H
#define DISKENCRYPTSETUP_P_H

#include "workers/baseencryptworker.h"

#include <QObject>

class DiskEncryptSetup;
class DiskEncryptSetupPrivate : public QObject
{
    Q_OBJECT
    friend class DiskEncryptSetup;
    DiskEncryptSetup *qptr { nullptr };
    bool jobRunning { false };

    explicit DiskEncryptSetupPrivate(DiskEncryptSetup *parent);
    void initialize();
    void resumeEncryption();
    bool checkAuth(const QString &action);
    bool validateInitArgs(const QVariantMap &args);
    bool validateResumeArgs(const QVariantMap &args);
    bool validateDecryptArgs(const QVariantMap &args);
    bool validateChgPwdArgs(const QVariantMap &args);

    QString resolveDeviceByDetachHeaderName(const QString &fileName);

    FILE_ENCRYPT_NS::BaseEncryptWorker *createInitWorker(const QString &type, const QVariantMap &args);
    FILE_ENCRYPT_NS::BaseEncryptWorker *createDecryptWorker(const QString &type, const QVariantMap &args);

    void initThreadConnection(const QThread *thread);

public Q_SLOTS:
    void onInitEncryptFinished();
    void onResumeEncryptFinished();
    void onDecryptFinished();
    void onPassphraseChanged();

    void onLongTimeJobStarted();
    void onLongTimeJobStopped();
};

#endif   // DISKENCRYPTSETUP_P_H
