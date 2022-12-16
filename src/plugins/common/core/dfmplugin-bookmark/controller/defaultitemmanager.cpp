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
#include "defaultitemmanager.h"
#include "defaultitemmanager_p.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/dialogmanager.h"

USING_IO_NAMESPACE

namespace dfmplugin_bookmark {

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DefaultItemManagerPrivate::DefaultItemManagerPrivate(DefaultItemManager *qq)
    : QObject(qq),
      q(qq)
{
}

DefaultItemManager *DefaultItemManager::instance()
{
    static DefaultItemManager instance;
    return &instance;
}

void DefaultItemManager::initDefaultItems()
{
    d->defaultItemInitOrder.clear();
    static QStringList defOrder = { "Home", "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads" };
    d->defaultPluginItem << "Trash"
                         << "Recent";
    for (int i = 0; i < defOrder.count(); i++) {
        const QString &nameKey = defOrder.at(i);
        const QString &displayName = SystemPathUtil::instance()->systemPathDisplayName(nameKey);
        BookmarkData bookmarkData;
        bookmarkData.name = nameKey;   //For default item, save the english name to config.
        QString path { SystemPathUtil::instance()->systemPath(nameKey) };
        QUrl url = { UrlRoute::pathToReal(path) };
        d->defaultItemUrls.insert(nameKey, url);
        bookmarkData.url = url;
        bookmarkData.isDefaultItem = true;
        bookmarkData.index = i;
        bookmarkData.transName = displayName;
        d->defaultItemInitOrder.append(bookmarkData);
    }
}

void DefaultItemManager::addPluginItem(const QVariantMap &args, bool notify)
{
    const QString &nameKey = args.value("Property_Key_NameKey").toString();

    if (args.contains("Property_Key_PluginItemData")) {
        const QVariantMap &itemData = args.value("Property_Key_PluginItemData").toMap();
        QVariantMap bookmarkData = d->pluginItemData.value(nameKey);
        bookmarkData.insert("Property_Key_PluginItemData", itemData);
        d->pluginItemData.insert(nameKey, bookmarkData);
        if (notify)
            Q_EMIT pluginItemDataAdded(bookmarkData.value("Property_Key_Url").toString(),
                                       nameKey,
                                       bookmarkData.value("Property_Key_IsDefaultItem").toBool(),
                                       bookmarkData.value("Property_Key_Index").toInt());
        return;
    }

    d->pluginItemData.insert(nameKey, args);
}

QMap<QString, QUrl> DefaultItemManager::defaultItemUrls()
{
    return d->defaultItemUrls;
}

QList<BookmarkData> DefaultItemManager::defaultItemInitOrder()
{
    return d->defaultItemInitOrder;
}

QMap<QString, QVariantMap> DefaultItemManager::pluginItemData()
{
    return d->pluginItemData;
}

bool DefaultItemManager::isDefaultItem(const BookmarkData &data)
{
    bool isNameKeyEqual = false;
    bool isUrlEqual = false;
    if (data.isDefaultItem) {
        isNameKeyEqual = DefaultItemManager::instance()->defaultItemUrls().keys().contains(data.name);
        return isNameKeyEqual;
    }
    isNameKeyEqual = DefaultItemManager::instance()->defaultItemUrls().keys().contains(data.name);
    isUrlEqual = DefaultItemManager::instance()->defaultItemUrls().values().contains(data.url);

    return isNameKeyEqual && isUrlEqual;
}

bool DefaultItemManager::isDefaultUrl(const BookmarkData &data)
{
    return DefaultItemManager::instance()->defaultItemUrls().values().contains(data.url);
}

bool DefaultItemManager::isDefaultPluginItem(const QString &name)
{
    return d->defaultPluginItem.contains(name);
}

BookmarkData DefaultItemManager::pluginItemDataToBookmark(const QVariantMap &data)
{
    const QString &nameKey = data.value("Property_Key_NameKey").toString();
    const QString &displayName = data.value("Property_Key_DisplayName").toString();
    const QUrl &url = data.value("Property_Key_Url").toUrl();
    int index = data.value("Property_Key_Index").toInt();
    bool isDefaultItem = data.value("Property_Key_IsDefaultItem").toBool();
    BookmarkData bookmarkData;
    bookmarkData.name = nameKey;
    bookmarkData.transName = displayName;
    bookmarkData.url = url;
    bookmarkData.index = index;
    bookmarkData.isDefaultItem = isDefaultItem;

    return bookmarkData;
}

DefaultItemManager::DefaultItemManager(QObject *parent)
    : QObject(parent), d(new DefaultItemManagerPrivate(this))
{
}
}
