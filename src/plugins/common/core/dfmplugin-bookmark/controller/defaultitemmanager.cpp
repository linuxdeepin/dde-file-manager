// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defaultitemmanager.h"
#include "defaultitemmanager_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/dialogmanager.h>

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

void DefaultItemManager::addPluginItem(const QVariantMap &args)
{
    const QString &nameKey = args.value("Property_Key_NameKey").toString();

    if (args.contains("Property_Key_PluginItemData")) {
        d->pluginItemData.insert(nameKey, args);
        Q_EMIT pluginItemDataAdded(args.value("Property_Key_Url").toString(),
                                   nameKey,
                                   args.value("Property_Key_IsDefaultItem").toBool(),
                                   args.value("Property_Key_Index").toInt());
    }
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
