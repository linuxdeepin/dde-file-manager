// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSCONTROLADAPTOR_H
#define ACCESSCONTROLADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.AccessControlManager
 */
class AccessControlAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.AccessControlManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.AccessControlManager\">\n"
"    <signal name=\"DiskPasswordChecked\">\n"
"      <arg direction=\"out\" type=\"i\" name=\"code\"/>\n"
"    </signal>\n"
"    <signal name=\"DiskPasswordChanged\">\n"
"      <arg direction=\"out\" type=\"i\" name=\"code\"/>\n"
"    </signal>\n"
"    <method name=\"ChangeDiskPassword\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"oldPwd\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"newPwd\"/>\n"
"    </method>\n"
"    <method name=\"Chmod\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"in\" type=\"u\" name=\"mode\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    AccessControlAdaptor(QObject *parent);
    virtual ~AccessControlAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void ChangeDiskPassword(const QString &oldPwd, const QString &newPwd);
    bool Chmod(const QString &path, uint mode);
Q_SIGNALS: // SIGNALS
    void DiskPasswordChanged(int code);
    void DiskPasswordChecked(int code);
};

#endif
