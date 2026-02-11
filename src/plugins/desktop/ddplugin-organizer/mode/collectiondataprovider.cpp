// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectiondataprovider.h"

#include <QDebug>
#include <QPair>

using namespace ddplugin_organizer;

CollectionDataProvider::CollectionDataProvider(QObject *parent) : QObject(parent)
{

}

CollectionDataProvider::~CollectionDataProvider()
{

}

QString CollectionDataProvider::key(const QUrl &url) const
{
    QString ret;
    auto it = std::find_if(collections.begin(), collections.end(), [&url](const CollectionBaseDataPtr &ptr) {
        return ptr->items.contains(url);
    });

    if (it != collections.end())
        ret = (*it)->key;

    return ret;
}

QString CollectionDataProvider::name(const QString &key) const
{
    QString ret;
    if (auto ptr = collections.value(key))
        ret = ptr->name;

    return ret;
}

QList<QString> CollectionDataProvider::keys() const
{
    return collections.keys();
}

QList<QUrl> CollectionDataProvider::items(const QString &key) const
{
    QList<QUrl> ret;
    if (auto ptr = collections.value(key))
        ret = ptr->items;

    return ret;
}

bool CollectionDataProvider::contains(const QString &key, const QUrl &url) const
{
    auto it = collections.find(key);
    if (it == collections.end()) {
        fmDebug() << "Collection not found:" << key;
        return false;
    }

    return (*it)->items.contains(url);
}

bool CollectionDataProvider::sorted(const QString &key, const QList<QUrl> &urls)
{
    auto it = collections.find(key);
    if (it == collections.end()) {
        fmWarning() << "Cannot sort: collection not found:" << key;
        return false;
    }

    if ((*it)->items.size() != urls.size()) {
        fmWarning() << "Cannot sort: size mismatch for collection" << key
                    << "existing:" << (*it)->items.size() << "new:" << urls.size();
        return false;
    }

    // check data, \a all member of urls must be in items.
    for (const QUrl &url : urls) {
        if (!(*it)->items.contains(url)) {
            fmWarning() << "Cannot sort: url not found in collection" << key << "url:" << url.toString();
            return false;
        }
    }

    (*it)->items = urls;
    fmInfo() << "Collection sorted successfully:" << key << "with" << urls.size() << "items";
    emit itemsChanged(key);
    return true;
}

void CollectionDataProvider::moveUrls(const QList<QUrl> &urls, const QString &targetKey, int targetIndex)
{
    if (urls.isEmpty()) {
        fmDebug() << "moveUrls called with empty url list";
        return;
    }

    auto sourceId = key(urls.first());
    if (sourceId.isEmpty()) {
        fmWarning() << "Source collection not found for urls, cannot move";
        return;
    }
    // In Qt6, when the size of the list is exceeded, the insert operation of QList will trigger an assertion crash
    // To avoid the crash, if the size bigger than list's size, use append instead.
    auto moveUrlInItems = [&urls](QHash<QString, CollectionBaseDataPtr>::Iterator &it, int targetIndex) {
        if (targetIndex > it.value()->items.size()) {
            for (auto url : urls) {
                it.value()->items.append(url);
            }
        } else {
            for (auto url : urls) {
                it.value()->items.insert(targetIndex++, url);
            }
        }
    };
    if (sourceId == targetKey) {
        // same collection
        auto it = collections.find(sourceId);
        if (it != collections.end()) {
            for (auto url : urls) {
                int oldIndex = it.value()->items.indexOf(url);
                if (-1 == oldIndex) {
                    fmWarning() << "Unknown error: url not found in collection" << url.toString() << "collection items:" << it.value()->items.size();
                    continue;
                }
                if (oldIndex < targetIndex)
                    targetIndex--;
                it.value()->items.removeOne(url);
            }
            moveUrlInItems(it, targetIndex);

            emit itemsChanged(sourceId);
        }
    } else {
        // collection to other collection
        auto it = collections.find(sourceId);
        if (it != collections.end()) {
            for (auto url : urls) {
                it.value()->items.removeOne(url);
            }
            emit itemsChanged(sourceId);
        } else {
            fmWarning() << "Cannot find source collection:" << sourceId;
        }
        it = collections.find(targetKey);
        if (it != collections.end()) {
            moveUrlInItems(it, targetIndex);
            emit itemsChanged(targetKey);
        }
    }
}

void CollectionDataProvider::addPreItems(const QString &targetKey, const QList<QUrl> &urls, int targetIndex)
{
    auto it = preCollectionItems.find(targetKey);
    if (it == preCollectionItems.end()) {
        QPair<int, QList<QUrl>> its{targetIndex, urls};
        preCollectionItems.insert(targetKey, its);
    } else {
        // merge to existing index
        it.value().second.append(urls);
    }
}

bool CollectionDataProvider::checkPreItem(const QUrl &url, QString &key, int &index)
{
    for (auto it = preCollectionItems.constBegin(); it != preCollectionItems.constEnd(); ++it) {
        if (it.value().second.contains(url)) {
            key = it.key();
            index = it.value().first;
            return true;
        }
    }

    return false;
}

bool CollectionDataProvider::takePreItem(const QUrl &url, QString &key, int &index)
{
    for (auto it = preCollectionItems.begin(); it != preCollectionItems.end(); ++it) {
        if (it.value().second.contains(url)) {
            key = it.key();
            // current index will to be used,add it
            index = it.value().first++;

            it.value().second.removeAll(url);
            if (it.value().second.isEmpty())
                preCollectionItems.remove(key);

            return true;
        }
    }

    return false;
}

