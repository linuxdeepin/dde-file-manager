// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGERADAPTOR_H
#define TAGMANAGERADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.Daemon.TagManager
 */
class TagManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.TagManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.Daemon.TagManager\">\n"
"    <signal name=\"TagsServiceReady\"/>\n"
"    <signal name=\"NewTagsAdded\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"tags\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"TagsDeleted\">\n"
"      <arg direction=\"out\" type=\"as\" name=\"tags\"/>\n"
"    </signal>\n"
"    <signal name=\"TagsColorChanged\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"oldAndNew\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"TagsNameChanged\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"oldAndNew\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"FilesTagged\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"fileAndTags\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"FilesUntagged\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"fileAndTags\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <method name=\"Query\">\n"
"      <arg direction=\"out\" type=\"v\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opt\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"Query\">\n"
"      <arg direction=\"out\" type=\"v\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opt\"/>\n"
"    </method>\n"
"    <method name=\"Insert\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opt\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"value\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In1\"/>\n"
"    </method>\n"
"    <method name=\"Delete\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opt\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"value\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In1\"/>\n"
"    </method>\n"
"    <method name=\"Update\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opt\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"value\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In1\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    TagManagerAdaptor(QObject *parent);
    virtual ~TagManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool Delete(int opt, const QVariantMap &value);
    bool Insert(int opt, const QVariantMap &value);
    QDBusVariant Query(int opt);
    QDBusVariant Query(int opt, const QStringList &value);
    bool Update(int opt, const QVariantMap &value);
Q_SIGNALS: // SIGNALS
    void FilesTagged(const QVariantMap &fileAndTags);
    void FilesUntagged(const QVariantMap &fileAndTags);
    void NewTagsAdded(const QVariantMap &tags);
    void TagsColorChanged(const QVariantMap &oldAndNew);
    void TagsDeleted(const QStringList &tags);
    void TagsNameChanged(const QVariantMap &oldAndNew);
    void TagsServiceReady();
};

#endif
