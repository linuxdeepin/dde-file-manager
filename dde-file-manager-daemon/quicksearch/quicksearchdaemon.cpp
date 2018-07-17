/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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

#include "quicksearchdaemon.h"
#include "quick_search/dquicksearch.h"
#include "dbusadaptor/quicksearchdaemon_adaptor.h"


#include <QDBusMetaType>
#include <QByteArrayList>


static constexpr const char *ObjectPath{"/com/deepin/filemanager/daemon/QuickSearchDaemon"};


QuickSearchDaemon::QuickSearchDaemon(QObject *const parent)
    : QObject{parent},
      adaptor{new QuickSearchDaemonAdaptor{ this }}
{
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QPair<QByteArray, QByteArray>>();
    qDBusRegisterMetaType<QList<QPair<QByteArray, QByteArray>>>();


    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("Failed to register object."); //###: log!
    }
}

QDBusVariant QuickSearchDaemon::createCache()
{
    bool flag{ DQuickSearch::instance()->createCache() };
    QDBusVariant dbus_var{ flag };

    return dbus_var;
}

QDBusVariant QuickSearchDaemon::whetherCacheCompletely()
{
    bool flag{ DQuickSearch::instance()->whetherCacheCompletely() };
    QDBusVariant dbus_var{ flag };

    return dbus_var;
}

QDBusVariant QuickSearchDaemon::search(const QDBusVariant &current_dir, const QDBusVariant &key_words)
{
    QVariant path_var{ current_dir.variant() };
    QVariant key_words_var{ key_words.variant() };

#ifdef QT_DEBUG
    qDebug() << path_var.toString();
    qDebug() << key_words_var.toString();
#endif //QT_DEBUG

    QVariant searched_result{ DQuickSearch::instance()->search(path_var.toString(), key_words_var.toString()) };
    QDBusVariant dbus_var{ searched_result };
    return dbus_var;
}

void QuickSearchDaemon::fileWereCreated(const QDBusVariant &file_list)
{
    QVariant variant{ file_list.variant() };
    QDBusArgument argument{ variant.value<QDBusArgument>() };
    QDBusArgument::ElementType current_type{ argument.currentType() };
    QList<QByteArray> list_byte_arrays{};

    if (current_type == QDBusArgument::ElementType::ArrayType) {
        argument >> list_byte_arrays;
    }

    DQuickSearch::instance()->filesWereCreated(list_byte_arrays);
}

void QuickSearchDaemon::fileWereDeleted(const QDBusVariant &file_list)
{
    QVariant variant{ file_list.variant() };
    QDBusArgument argument{ variant.value<QDBusArgument>() };
    QDBusArgument::ElementType current_type{ argument.currentType() };
    QList<QByteArray> list_byte_arrays{};

    if (current_type == QDBusArgument::ElementType::ArrayType) {
        argument >> list_byte_arrays;
    }

    DQuickSearch::instance()->filesWereDeleted(list_byte_arrays);
}

void QuickSearchDaemon::fileWereRenamed(const QDBusVariant &file_list)
{
    QVariant variant{ file_list.variant() };
    QDBusArgument argument{ variant.value<QDBusArgument>() };
    QDBusArgument::ElementType current_type{ argument.currentType() };
    QList<QPair<QByteArray, QByteArray>> list_byte_arrays{};

    if (current_type == QDBusArgument::ElementType::ArrayType) {
        argument >> list_byte_arrays;
    }

    DQuickSearch::instance()->filesWereRenamed(list_byte_arrays);
}


