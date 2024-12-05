// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ENCRYPTWORKER_H
#define ENCRYPTWORKER_H

#include "diskencrypt_global.h"

#include <QThread>
#include <QMutex>
#include <QReadWriteLock>

FILE_ENCRYPT_BEGIN_NS
#define TOKEN_FILE_PATH QString("/tmp/%1_tpm_token.json")

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(const QString &jobID, QObject *parent = nullptr)
        : QThread(parent), jobID(jobID) {}

    inline int exitError()
    {
        QMutexLocker locker(&mtx);
        return exitCode;
    }

protected:
    inline void setExitCode(int code)
    {
        QMutexLocker locker(&mtx);
        exitCode = code;
    }

    static QString decryptPasswd(const QString &passwd);

protected:
    int exitCode { disk_encrypt::kSuccess };
    QString jobID;
    QMutex mtx;
};

class PrencryptWorker : public Worker
{
    Q_OBJECT
public:
    explicit PrencryptWorker(const QString &jobID,
                             const QVariantMap &params,
                             QObject *parent);
    int cipherPos() const { return keyslotCipher; }
    int recKeyPos() const { return keyslotRecKey; }

protected:
    void run() override;
    int writeEncryptParams(const QString &device = QString());
    int setFstabTimeout();

private:
    QVariantMap params;
    int keyslotCipher { -1 };
    int keyslotRecKey { -1 };
};

class ReencryptWorkerV2 : public Worker
{
    Q_OBJECT

public:
    explicit ReencryptWorkerV2(QObject *parent = nullptr);
    void setEncryptParams(const QVariantMap &params);
    void loadReencryptConfig(const QString &device = QString());
    disk_encrypt::EncryptConfig encryptConfig() const;
    void ignoreParamRequest();
    static bool setFsPassno(const QString &uuid, const QString &state);

Q_SIGNALS:
    void requestEncryptParams(const QVariantMap &encConf);
    void deviceReencryptResult(const QString &device,
                               int result,
                               const QString &msg);

protected:
    void run() override;
    bool hasUnfinishedOnlineEncryption();
    void setPassphrase();
    void setRecoveryKey(QString *key, bool *expKey);
    void setBakcingDevLabel();
    void updateCrypttab();
    void removeEncryptFile();
    QString updateTokenKeyslots(const QString &token, int keyslot);
    bool validateParams();
    void disableABRecovery();
    int waitForInput();

private:
    QVariantMap params;
    QReadWriteLock lockParam;

    QReadWriteLock lockRequest;
    bool ignoreRequest { false };

    disk_encrypt::EncryptConfig config;
};

class DecryptWorker : public Worker
{
    Q_OBJECT
public:
    explicit DecryptWorker(const QString &jobID,
                           const QVariantMap &params,
                           QObject *parent = nullptr);

protected:
    void run() override;
    int writeDecryptParams();
    QString findEncryptSrcDev(const QString &activeName);

private:
    QVariantMap params;
};

class ChgPassWorker : public Worker
{
    Q_OBJECT
public:
    explicit ChgPassWorker(const QString &jobID,
                           const QVariantMap &params,
                           QObject *parent = nullptr);

protected:
    void run() override;

private:
    QVariantMap params;
};

FILE_ENCRYPT_END_NS

#endif   // ENCRYPTWORKER_H
