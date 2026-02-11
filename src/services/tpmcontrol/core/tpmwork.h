// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TPMWORK_H
#define TPMWORK_H

#include "service_tpmcontrol_global.h"

#include <QObject>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
class QLibrary;
QT_END_NAMESPACE

SERVICETPMCONTROL_BEGIN_NAMESPACE

class TPMWork : public QObject
{
    Q_OBJECT
public:
    explicit TPMWork(QObject *parent = nullptr);
    ~TPMWork();

    // Pro APIs (using tpm2-tools)
    int isTPMAvailable();
    int checkTPMLockout();
    int getRandom(int size, QString *output);
    int isSupportAlgo(const QString &algoName, bool *support);
    int encrypt(const QVariantMap &params);
    int decrypt(const QVariantMap &params, QString *pwd);
    int ownerAuthStatus();

    bool isLibraryLoaded() const;

private:
    QLibrary *tpmLib { nullptr };
};

SERVICETPMCONTROL_END_NAMESPACE

#endif   // TPMWORK_H
