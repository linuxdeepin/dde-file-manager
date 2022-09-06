// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULT_ADAPTOR_H
#define VAULT_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "../vault/vaultmanager.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.VaultManager
 */
class VaultAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.VaultManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.VaultManager\">\n"
"    <method name=\"sysUserChanged\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"curUser\"/>\n"
"    </method>\n"
"    <method name=\"setRefreshTime\">\n"
"      <arg direction=\"in\" type=\"t\" name=\"time\"/>\n"
"    </method>\n"
"    <method name=\"getLastestTime\">\n"
"      <arg direction=\"out\" type=\"t\"/>\n"
"    </method>\n"
"    <method name=\"getSelfTime\">\n"
"      <arg direction=\"out\" type=\"t\"/>\n"
"    </method>\n"
"    <method name=\"checkAuthentication\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"type\"/>\n"
"    </method>\n"
"    <method name=\"isLockEventTriggered\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"triggerLockEvent\"/>\n"
"    <method name=\"clearLockEvent\"/>\n"
"  </interface>\n"
        "")
public:
    VaultAdaptor(VaultManager *parent);
    virtual ~VaultAdaptor();

    inline VaultManager *parent() const
    { return static_cast<VaultManager *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool checkAuthentication(const QString &type);
    void clearLockEvent();
    qulonglong getLastestTime();
    qulonglong getSelfTime();
    bool isLockEventTriggered();
    void setRefreshTime(qulonglong time);
    void sysUserChanged(const QString &curUser);
    void triggerLockEvent();
Q_SIGNALS: // SIGNALS
};

#endif
