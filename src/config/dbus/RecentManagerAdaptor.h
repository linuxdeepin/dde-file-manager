// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMANAGERADAPTOR_H
#define RECENTMANAGERADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.Daemon.RecentManager
 */
class RecentManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.RecentManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.Daemon.RecentManager\">\n"
"    <signal name=\"ReloadFinished\">\n"
"      <arg direction=\"out\" type=\"x\" name=\"timestamp\"/>\n"
"    </signal>\n"
"    <signal name=\"PurgeFinished\"/>\n"
"    <signal name=\"ItemAdded\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"href\"/>\n"
"      <arg direction=\"out\" type=\"x\" name=\"modified\"/>\n"
"    </signal>\n"
"    <signal name=\"ItemsRemoved\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"paths\"/>\n"
"    </signal>\n"
"    <signal name=\"ItemChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"path\"/>\n"
"      <arg direction=\"out\" type=\"x\" name=\"modified\"/>\n"
"    </signal>\n"
"    <method name=\"Reload\">\n"
"      <arg direction=\"out\" type=\"x\"/>\n"
"    </method>\n"
"    <method name=\"AddItem\">\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"item\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"RemoveItems\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"hrefs\"/>\n"
"    </method>\n"
"    <method name=\"PurgeItems\"/>\n"
"    <method name=\"GetItemsPath\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"GetItemsInfo\">\n"
"      <arg direction=\"out\" type=\"av\"/>\n"
"    </method>\n"
"    <method name=\"GetItemInfo\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"path\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    RecentManagerAdaptor(QObject *parent);
    virtual ~RecentManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void AddItem(const QVariantMap &item);
    QVariantMap GetItemInfo(const QString &path);
    QVariantList GetItemsInfo();
    QStringList GetItemsPath();
    void PurgeItems();
    qlonglong Reload();
    void RemoveItems(const QStringList &hrefs);
Q_SIGNALS: // SIGNALS
    void ItemAdded(const QString &path, const QString &href, qlonglong modified);
    void ItemChanged(const QString &path, qlonglong modified);
    void ItemsRemoved(const QStringList &paths);
    void PurgeFinished();
    void ReloadFinished(qlonglong timestamp);
};

#endif
