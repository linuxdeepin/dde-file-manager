// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTBRUTEFORCEPREVENTION_ADAPTOR_H
#define VAULTBRUTEFORCEPREVENTION_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "../vault/vaultbruteforceprevention.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.VaultManager2
 */
class VaultBruteForcePreventionAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.VaultManager2")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.VaultManager2\">\n"
"    <method name=\"getLeftoverErrorInputTimes\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"leftoverErrorInputTimesMinusOne\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"restoreLeftoverErrorInputTimes\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"startTimerOfRestorePasswordInput\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"getNeedWaitMinutes\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"    <method name=\"restoreNeedWaitMinutes\">\n"
"      <arg direction=\"in\" type=\"i\" name=\"userID\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    VaultBruteForcePreventionAdaptor(VaultBruteForcePrevention *parent);
    virtual ~VaultBruteForcePreventionAdaptor();

    inline VaultBruteForcePrevention *parent() const
    { return static_cast<VaultBruteForcePrevention *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    int getLeftoverErrorInputTimes(int userID);
    int getNeedWaitMinutes(int userID);
    void leftoverErrorInputTimesMinusOne(int userID);
    void restoreLeftoverErrorInputTimes(int userID);
    void restoreNeedWaitMinutes(int userID);
    void startTimerOfRestorePasswordInput(int userID);
Q_SIGNALS: // SIGNALS
};

#endif
