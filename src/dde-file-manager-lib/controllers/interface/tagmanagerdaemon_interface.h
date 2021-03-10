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
