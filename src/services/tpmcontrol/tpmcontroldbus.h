// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TPMCONTROLDBUS_H
#define TPMCONTROLDBUS_H

#include "service_tpmcontrol_global.h"

#include <QObject>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>

SERVICETPMCONTROL_BEGIN_NAMESPACE
class TPMWork;
SERVICETPMCONTROL_END_NAMESPACE

class TPMControlDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.TPMControl")

public:
    explicit TPMControlDBus(const char *name, QObject *parent = nullptr);
    ~TPMControlDBus();

public slots:
    // Simple query methods (no sensitive data)
    Q_SCRIPTABLE int IsTPMAvailable();
    Q_SCRIPTABLE int CheckTPMLockout();
    Q_SCRIPTABLE int IsSupportAlgo(const QString &algoName, bool &support);
    Q_SCRIPTABLE int OwnerAuthStatus();

    // Sensitive data methods (using file descriptors)
    Q_SCRIPTABLE int GetRandom(int size, QDBusUnixFileDescriptor &randomData);
    Q_SCRIPTABLE int Encrypt(const QDBusUnixFileDescriptor &params);
    Q_SCRIPTABLE int Decrypt(const QDBusUnixFileDescriptor &params, QDBusUnixFileDescriptor &password);

private:
    bool checkAuthentication(const QString &actionId);
    bool parseCredentialsFromFd(const QDBusUnixFileDescriptor &fd, QVariantMap *args);
    bool sendDataViaFd(const QByteArray &data, QDBusUnixFileDescriptor &fd);

    QScopedPointer<SERVICETPMCONTROL_NAMESPACE::TPMWork> m_tpmWork;
};

#endif   // TPMCONTROLDBUS_H
