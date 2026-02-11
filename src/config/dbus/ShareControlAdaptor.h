// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHARECONTROLADAPTOR_H
#define SHARECONTROLADAPTOR_H

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
 * Adaptor class for interface org.deepin.Filemanager.UserShareManager
 */
class ShareControlAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.UserShareManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.UserShareManager\">\n"
"    <method name=\"CloseSmbShareByShareName\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"show\"/>\n"
"    </method>\n"
"    <method name=\"SetUserSharePassword\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"passwd\"/>\n"
"    </method>\n"
"    <method name=\"EnableSmbServices\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"IsUserSharePasswordSet\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"username\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ShareControlAdaptor(QObject *parent);
    virtual ~ShareControlAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    bool CloseSmbShareByShareName(const QString &name, bool show);
    bool EnableSmbServices();
    bool IsUserSharePasswordSet(const QString &username);
    bool SetUserSharePassword(const QString &name, const QString &passwd);
Q_SIGNALS: // SIGNALS
};

#endif
