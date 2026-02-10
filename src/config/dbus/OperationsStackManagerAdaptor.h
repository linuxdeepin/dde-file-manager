// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATIONSSTACKMANAGERADAPTOR_H
#define OPERATIONSSTACKMANAGERADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.Daemon.OperationsStackManager
 */
class OperationsStackManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.OperationsStackManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.Daemon.OperationsStackManager\">\n"
"    <method name=\"SaveOperations\">\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"values\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"CleanOperations\"/>\n"
"    <method name=\"RevocationOperations\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"SaveRedoOperations\">\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"values\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"RevocationRedoOperations\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </method>\n"
"    <method name=\"CleanOperationsByUrl\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"urls\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    OperationsStackManagerAdaptor(QObject *parent);
    virtual ~OperationsStackManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void CleanOperations();
    void CleanOperationsByUrl(const QStringList &urls);
    QVariantMap RevocationOperations();
    QVariantMap RevocationRedoOperations();
    void SaveOperations(const QVariantMap &values);
    void SaveRedoOperations(const QVariantMap &values);
Q_SIGNALS: // SIGNALS
};

#endif
