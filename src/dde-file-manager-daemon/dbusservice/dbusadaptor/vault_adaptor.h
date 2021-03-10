/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
