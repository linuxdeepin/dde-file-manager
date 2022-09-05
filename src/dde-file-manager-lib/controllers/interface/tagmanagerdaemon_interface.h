// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGMANAGERDAEMON_INTERFACE_H
#define TAGMANAGERDAEMON_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.filemanager.daemon.TagManagerDaemon
 */
class TagManagerDaemonInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.filemanager.daemon.TagManagerDaemon"; }

public:
    TagManagerDaemonInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~TagManagerDaemonInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QDBusVariant> disposeClientData(const QVariantMap &filesAndTags, qulonglong type)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(filesAndTags) << QVariant::fromValue(type);
        return asyncCallWithArgumentList(QStringLiteral("disposeClientData"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void addNewTags(const QDBusVariant &new_tags);
    void changeTagColor(const QVariantMap &old_and_new_color);
    void changeTagName(const QVariantMap &old_and_new_name);
    void deleteTags(const QDBusVariant &be_deleted_tags);
    void filesWereTagged(const QVariantMap &tagged_files);
    void untagFiles(const QVariantMap &file_be_removed_tags);
};

namespace com {
  namespace deepin {
    namespace filemanager {
      namespace daemon {
        typedef ::TagManagerDaemonInterface TagManagerDaemon;
      }
    }
  }
}
#endif
