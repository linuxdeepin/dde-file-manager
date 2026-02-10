// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESUMEENCRYPTWORKER_H
#define RESUMEENCRYPTWORKER_H

#include "baseencryptworker.h"
#include "helpers/jobfilehelper.h"

#include <QReadWriteLock>

FILE_ENCRYPT_BEGIN_NS

struct AuthInfo
{
    QString device;
    QString passphrase;
    QString tpmToken;
    QString recoveryPath;
    QString recoveryKey;
};

class ResumeEncryptWorker : public BaseEncryptWorker
{
    Q_OBJECT
public:
    explicit ResumeEncryptWorker(const QVariantMap &args,
                                 QObject *parent = nullptr);
    QString recoveryKey();

Q_SIGNALS:
    void requestAuthInfo(const QVariantMap &devInfo);

private Q_SLOTS:
    void setAuthInfo(const QVariantMap &args);
    void ignoreAuthRequest();

protected:
    void run() override;

    bool waitForAuthInfo();
    void setPassphrase();
    void setRecoveryKey();
    void setPhyDevLabel();
    void updateCryptTab();
    void saveRecoveryKey();

    void loadJobFromDevice();

private:
    bool m_ignoreFlag { false };
    AuthInfo m_authArgs;
    job_file_helper::JobDescArgs m_jobArgs;
    QReadWriteLock m_lock;
};

FILE_ENCRYPT_END_NS
#endif   // RESUMEENCRYPTWORKER_H
