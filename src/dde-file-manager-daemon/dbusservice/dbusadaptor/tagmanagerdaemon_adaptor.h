/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef TAGMANAGERDAEMON_ADAPTOR_H
#define TAGMANAGERDAEMON_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "tag/tagmanagerdaemon.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.TagManagerDaemon
 */
class TagManagerDaemonAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.TagManagerDaemon")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.TagManagerDaemon\">\n"
"    <method name=\"disposeClientData\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"filesAndTags\"/>\n"
"      <arg direction=\"in\" type=\"t\" name=\"type\"/>\n"
"      <arg direction=\"out\" type=\"v\" name=\"result\"/>\n"
"    </method>\n"
"    <signal name=\"filesWereTagged\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"tagged_files\"/>\n"
"    </signal>\n"
"    <signal name=\"untagFiles\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"file_be_removed_tags\"/>\n"
"    </signal>\n"
"    <signal name=\"addNewTags\">\n"
"      <arg direction=\"out\" type=\"v\" name=\"new_tags\"/>\n"
"    </signal>\n"
"    <signal name=\"deleteTags\">\n"
"      <arg direction=\"out\" type=\"v\" name=\"be_deleted_tags\"/>\n"
"    </signal>\n"
"    <signal name=\"changeTagName\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"old_and_new_name\"/>\n"
"    </signal>\n"
"    <signal name=\"changeTagColor\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"old_and_new_color\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    TagManagerDaemonAdaptor(QObject *parent);
    virtual ~TagManagerDaemonAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QDBusVariant disposeClientData(const QVariantMap &filesAndTags, qulonglong type);
Q_SIGNALS: // SIGNALS
    void addNewTags(const QDBusVariant &new_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void deleteTags(const QDBusVariant &be_deleted_tags);
    void filesWereTagged(const QVariantMap &tagged_files);
    void untagFiles(const QVariantMap &file_be_removed_tags);
};

#endif
