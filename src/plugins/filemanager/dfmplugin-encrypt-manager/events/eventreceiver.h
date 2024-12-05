// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "encrypt_manager_global.h"

#include <QObject>

namespace dfmplugin_encrypt_manager {

class EventReceiver : public QObject
{
    Q_OBJECT
public:
    static EventReceiver *instance();

public Q_SLOTS:
    bool tpmIsAvailable();
    bool getRandomByTpm(int size, QString *output);
    bool isTpmSupportAlgo(const QString &algoName, bool *support);
    bool encrypyByTpm(const QString &hashAlgo, const QString &keyAlgo,
                      const QString &keyPin, const QString &password,
                      const QString &dirPath);
    bool decryptByTpm(const QString &keyPin, const QString &dirPath, QString *pwd);

    int tpmIsAvailableProcess();
    int getRandomByTpmProcess(int size, QString *output);
    int isTpmSupportAlgoProcess(const QString &algoName, bool *support);
    int encryptByTpmProcess(const QVariantMap &encryptParams);
    int decryptByTpmProcess(const QVariantMap &decryptParams, QString *pwd);
    int ownerAuthStatus();

private:
    explicit EventReceiver(QObject *parent = nullptr);
    void initConnection();
};
}

#endif   // EVENTRECEIVER_H
