// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXADAPTOR_H
#define TEXTINDEXADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.TextIndex
 */
class TextIndexAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.TextIndex")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.TextIndex\">\n"
"    <signal name=\"TaskFinished\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"type\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"success\"/>\n"
"    </signal>\n"
"    <signal name=\"TaskProgressChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"type\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"out\" type=\"x\" name=\"count\"/>\n"
"    </signal>\n"
"    <method name=\"CreateIndexTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"    </method>\n"
"    <method name=\"UpdateIndexTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"    </method>\n"
"    <method name=\"RemoveIndexTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"paths\"/>\n"
"    </method>\n"
"    <method name=\"StopCurrentTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"HasRunningTask\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"IndexDatabaseExists\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"GetLastUpdateTime\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    TextIndexAdaptor(QObject *parent);
    virtual ~TextIndexAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool CreateIndexTask(const QString &path);
    QString GetLastUpdateTime();
    bool HasRunningTask();
    bool IndexDatabaseExists();
    bool RemoveIndexTask(const QStringList &paths);
    bool StopCurrentTask();
    bool UpdateIndexTask(const QString &path);
Q_SIGNALS: // SIGNALS
    void TaskFinished(const QString &type, const QString &path, bool success);
    void TaskProgressChanged(const QString &type, const QString &path, qlonglong count);
};

#endif
