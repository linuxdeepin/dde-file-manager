// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TPMWORK_H
#define TPMWORK_H

#include "encrypt_manager_global.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QLibrary;
QT_END_NAMESPACE

namespace dfmplugin_encrypt_manager {

class TPMWork : public QObject
{
    Q_OBJECT
public:
    explicit TPMWork(QObject *parent = nullptr);
    ~TPMWork();
    bool checkTPMAvailable();
    bool getRandom(int size, QString *output);
    bool isSupportAlgo(const QString &algoName, bool *support);
    bool encrypt(const QString &hashAlgo, const QString &keyAlgo,
                 const QString &keyPin, const QString &password,
                 const QString &dirPath);
    bool decrypt(const QString &keyPin, const QString &dirPath, QString *psw);

    int checkTPMAvailbableByTools();
    int getRandomByTools(int size, QString *output);
    int isSupportAlgoByTools(const QString &algoName, bool *support);
    int encryptByTools(const EncryptParams &params);
    int decryptByTools(const DecryptParams &params, QString *pwd);
    int ownerAuthStatus();

private:
    bool initTpm2(const QString &hashAlgo, const QString &keyAlgo,
                  const QString &keyPin, const QString &dirPath);

    QLibrary *tpmLib { nullptr };
};

}

#endif   // TPMWORK_H
