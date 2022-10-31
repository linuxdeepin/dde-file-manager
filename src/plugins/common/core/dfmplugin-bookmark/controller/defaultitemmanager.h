/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu <zhuangshu@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef DEFAULTITEMMANAGER_H
#define DEFAULTITEMMANAGER_H

#include "dfmplugin_bookmark_global.h"

#include "bookmarkmanager.h"

#include <QObject>

namespace dfmplugin_bookmark {

class DefaultItemManagerPrivate;
class DefaultItemManager : public QObject
{
    Q_OBJECT
    friend class DefaultItemManagerPrivate;
    DefaultItemManagerPrivate *const d = nullptr;

public:
    static DefaultItemManager *instance();
    void initDefaultItems();
    void addPluginItem(const QVariantMap &args);
    QMap<QString, QUrl> defaultItemUrls();
    QList<BookmarkData> defaultItemInitOrder();
    QMap<QString, QVariantMap> pluginItemData();
    bool isDefaultItem(const BookmarkData &data);
    bool isDefaultUrl(const BookmarkData &data);
    bool isDefaultPluginItem(const QString &name);
    BookmarkData pluginItemDataToBookmark(const QVariantMap &data);

private:
    explicit DefaultItemManager(QObject *parent = nullptr);
};

}

#endif   // DEFAULTITEMMANAGER_H
