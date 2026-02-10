// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTMANAGERADAPTOR_H
#define VAULTMANAGERADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.Filemanager.Daemon.VaultManager
 */
class VaultManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.VaultManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.Daemon.VaultManager\">\n"
"    <signal name=\"LockEventTriggered\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"user\"/>\n"
"    </signal>\n"
"    <signal name=\"ChangedVaultState\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"map\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <method name=\"SysUserChanged\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"curUser\"/>\n"
"    </method>\n"
"    <method name=\"SetRefreshTime\">\n"
"      <arg direction=\"in\" type=\"t\" name=\"time\"/>\n"
"    </method>\n"
"    <method name=\"GetLastestTime\">\n"
"      <arg direction=\"out\" type=\"t\"/>\n"
"    </method>\n"
"    <method name=\"GetSelfTime\">\n"
"      <arg direction=\"out\" type=\"t\"/>\n"
"    </method>\n"
"    <method name=\"IsLockEventTriggered\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"TriggerLockEvent\"/>\n"
"    <method name=\"ClearLockEvent\"/>\n"
"    <method name=\"ComputerSleep\">\n"
"      <arg direction=\"in\" type=\"b\" name=\"bSleep\"/>\n"
"    </method>\n"
"    <method name=\"GetLeftoverErrorInputTimes\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"LeftoverErrorInputTimesMinusOne\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"RestoreLeftoverErrorInputTimes\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"StartTimerOfRestorePasswordInput\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"GetNeedWaitMinutes\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"RestoreNeedWaitMinutes\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    VaultManagerAdaptor(QObject *parent);
    virtual ~VaultManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ClearLockEvent();
    void ComputerSleep(bool bSleep);
    qulonglong GetLastestTime();
    int GetLeftoverErrorInputTimes(int userID);
    int GetNeedWaitMinutes(int userID);
    qulonglong GetSelfTime();
    bool IsLockEventTriggered();
    void LeftoverErrorInputTimesMinusOne(int userID);
    void RestoreLeftoverErrorInputTimes(int userID);
    void RestoreNeedWaitMinutes(int userID);
    void SetRefreshTime(qulonglong time);
    void StartTimerOfRestorePasswordInput(int userID);
    void SysUserChanged(const QString &curUser);
    void TriggerLockEvent();
Q_SIGNALS: // SIGNALS
    void ChangedVaultState(const QVariantMap &map);
    void LockEventTriggered(const QString &user);
};

#endif
