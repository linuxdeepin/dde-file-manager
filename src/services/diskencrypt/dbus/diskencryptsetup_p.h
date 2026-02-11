// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKENCRYPTSETUP_P_H
#define DISKENCRYPTSETUP_P_H

#include "workers/baseencryptworker.h"

#include <QObject>
#include <QDBusUnixFileDescriptor>
#include <QVariantMap>

namespace Dtk {
namespace Core {
class DConfig;
}
}

class DiskEncryptSetup;
class DiskEncryptSetupPrivate : public QObject
{
    Q_OBJECT
    friend class DiskEncryptSetup;
    DiskEncryptSetup *qptr { nullptr };
    bool jobRunning { false };
    Dtk::Core::DConfig *config { nullptr };

    // Config change handling state - prevent concurrent operations
    bool isHandlingConfigChange { false };     // 是否正在处理配置变更
    bool currentTargetValue { false };         // 当前正在执行的目标值
    bool hasPendingConfigChange { false };     // 是否有待处理的变更
    bool pendingTargetValue { false };         // 待处理的目标值

    explicit DiskEncryptSetupPrivate(DiskEncryptSetup *parent);
    void initialize();
    void setupConfigWatcher();
    void resumeEncryption(const QVariantMap &args = QVariantMap());
    bool checkAuth(const QString &action);
    bool validateInitArgs(const QVariantMap &args);
    bool validateResumeArgs(const QVariantMap &args);
    bool validateDecryptArgs(const QVariantMap &args);
    bool validateChgPwdArgs(const QVariantMap &args);

    // Parse credentials from file descriptor
    bool parseCredentialsFromFd(const QDBusUnixFileDescriptor &credentialsFd, QVariantMap *args);

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

    void onConfigValueChanged(const QString &key);
    void onOverlayDMModeChangeFinished(bool success, bool targetValue);

private:
    bool handleOverlayDMModeChange(bool enabled);
    void handleOverlayDMModeChangeAsync(bool enabled);
    void processPendingConfigChange();
    void syncConfigWithFileSystem();
    bool createOverlayDMFlagFile();
    bool removeOverlayDMFlagFile();
    bool updateInitramfs();
};

#endif   // DISKENCRYPTSETUP_P_H
