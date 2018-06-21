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


static constexpr const char *ObjectPath{"/com/deepin/filemanager/daemon/QuickSearchDaemon"};


QuickSearchDaemon::QuickSearchDaemon(QObject *const parent)
    : QObject{ parent },
      adaptor{ new QuickSearchDaemonAdaptor{ this } }
{
    if (!QDBusConnection::systemBus().registerObject(ObjectPath, this)) {
        qFatal("Failed to register object."); //###: log!
    }
}

QDBusVariant QuickSearchDaemon::search(const QDBusVariant &path, const QDBusVariant &key_words)
{
    QVariant path_var{ path.variant() };
    QVariant key_words_var{ key_words.variant() };
    QVariant searched_result{ DQuickSearch::instance()->search(path_var.toString(), key_words_var.toString()) };
    QDBusVariant dbus_var{ searched_result };

    return dbus_var;
}


