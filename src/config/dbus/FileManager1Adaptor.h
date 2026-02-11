// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGER1ADAPTOR_H
#define FILEMANAGER1ADAPTOR_H

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
 * Adaptor class for interface org.freedesktop.FileManager1
 */
class FileManager1Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.FileManager1\">\n"
"    <method name=\"ShowFolders\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"URIs\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"StartupId\"/>\n"
"    </method>\n"
"    <method name=\"ShowItemProperties\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"URIs\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"StartupId\"/>\n"
"    </method>\n"
"    <method name=\"ShowItems\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"URIs\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"StartupId\"/>\n"
"    </method>\n"
"    <method name=\"Trash\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"URIs\"/>\n"
"    </method>\n"
"    <method name=\"Open\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"Args\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    FileManager1Adaptor(QObject *parent);
    virtual ~FileManager1Adaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Open(const QStringList &Args);
    void ShowFolders(const QStringList &URIs, const QString &StartupId);
    void ShowItemProperties(const QStringList &URIs, const QString &StartupId);
    void ShowItems(const QStringList &URIs, const QString &StartupId);
    void Trash(const QStringList &URIs);
Q_SIGNALS: // SIGNALS
};

#endif
