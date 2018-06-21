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

#include "dquicksearchcontroller.h"


static constexpr const  char *const service{ "com.deepin.filemanager.daemon" };
static constexpr const char *const path{ "/com/deepin/filemanager/daemon/QuickSearchDaemon" };

DQuickSearchController::DQuickSearchController(QObject *const parent)
    : QObject{ parent }
{
    m_interface = std::unique_ptr<QuickSearchDaemonInterface> { new QuickSearchDaemonInterface{
            service,
            path,
            QDBusConnection::systemBus()
        }
    };
}

QList<QString> DQuickSearchController::search(const QString &local_path, const QString &key_words)
{
    QDBusVariant searched_list{ m_interface->search(QDBusVariant{local_path}, QDBusVariant{key_words}) };
    QVariant list_var{ searched_list.variant() };

    return list_var.toStringList();
}


