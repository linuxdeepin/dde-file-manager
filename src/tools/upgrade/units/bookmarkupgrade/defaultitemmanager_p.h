/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#ifndef DEFAULTITEMMANAGER_P_H
#define DEFAULTITEMMANAGER_P_H

#include "units/bookmarkupgradeunit.h"
#include <QObject>
#include <QMap>
#include <QList>

namespace dfm_upgrade {
class DefaultItemManager;
class DefaultItemManagerPrivate : public QObject
{
    Q_OBJECT
    friend class DefaultItemManager;
    DefaultItemManager *const q;

    explicit DefaultItemManagerPrivate(DefaultItemManager *qq);

    QList<BookmarkData> defaultItemInitOrder;
    QMap<QString, QVariantMap> pluginItemData;
    QStringList defaultPluginItem;
};

}

#endif   // SIDEBARVIEW_P_H
