// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTCONTROLADAPTOR_H
#define MOUNTCONTROLADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.MountControl
 */
class MountControlAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.MountControl")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.MountControl\">\n"
"    <method name=\"Mount\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"opts\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In1\"/>\n"
"    </method>\n"
"    <method name=\"Unmount\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"opts\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In1\"/>\n"
"    </method>\n"
"    <method name=\"SupportedFileSystems\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    MountControlAdaptor(QObject *parent);
    virtual ~MountControlAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QVariantMap Mount(const QString &path, const QVariantMap &opts);
    QStringList SupportedFileSystems();
    QVariantMap Unmount(const QString &path, const QVariantMap &opts);
Q_SIGNALS: // SIGNALS
};

#endif
