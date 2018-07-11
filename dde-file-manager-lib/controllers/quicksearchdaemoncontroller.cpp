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


#include <dfileinfo.h>


#include "quicksearchdaemoncontroller.h"

static constexpr const  char *const service{ "com.deepin.filemanager.daemon" };
static constexpr const char *const path{ "/com/deepin/filemanager/daemon/QuickSearchDaemon" };


QuickSearchDaemonController::QuickSearchDaemonController(QObject *const parent)
    : QObject{ parent },
      interface_ptr{ nullptr }
{
    interface_ptr = std::unique_ptr<QuickSearchDaemonInterface> { new QuickSearchDaemonInterface{ service, path,
                QDBusConnection::systemBus(), nullptr }
    };
}

bool QuickSearchDaemonController::whetherCacheCompletely() const noexcept
{
    QDBusVariant dbus_var{ interface_ptr->whetherCacheCompletely() };
    QVariant result_var{ dbus_var.variant() };

    return result_var.toBool();
}

bool QuickSearchDaemonController::createCache() const noexcept
{
    QDBusVariant dbus_var{ interface_ptr->createCache() };
    QVariant result_var{ dbus_var.variant() };

    return result_var.toBool();
}

QList<QString> QuickSearchDaemonController::search(const QString &path_for_searching, const QString &key)
{
    QList<QString> result_list{};
    QFileInfo file_info{ path_for_searching };

    if (QFileInfo::exists(path_for_searching) && file_info.isDir()) {
        QDBusVariant var_local_file{ QVariant{path_for_searching} };
        QDBusVariant var_key{QVariant{ key }};
        QDBusVariant result{interface_ptr->search(var_local_file, var_key)};

        result_list = result.variant().toStringList();
    }

    return result_list;
}
