// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    if (it == collections.end())
        return false;

    return (*it)->items.contains(url);
}

bool CollectionDataProvider::sorted(const QString &key, const QList<QUrl> &urls)
{
    auto it = collections.find(key);
    if (it == collections.end())
        return false;

    if ((*it)->items.size() != urls.size())
        return false;

    // check data, \a all member of urls must be in items.
    for (const QUrl &url : urls) {
        if (!(*it)->items.contains(url))
            return false;
    }

    (*it)->items = urls;
    emit itemsChanged(key);
    return true;
}

void CollectionDataProvider::moveUrls(const QList<QUrl> &urls, const QString &targetKey, int targetIndex)
{
    if (urls.isEmpty())
        return;

    auto sourceId = key(urls.first());
    if (sourceId.isEmpty()) {
        // not belong collection
        return;
    }

    if (sourceId == targetKey) {
        // same collection
        auto it = collections.find(sourceId);
        if (it != collections.end()) {
            for (auto url : urls) {
                int oldIndex = it.value()->items.indexOf(url);
                if (-1 == oldIndex) {
                    fmWarning() << "unknow error:" << url << it.value()->items;
                    continue;
                }
                if (oldIndex < targetIndex)
                    targetIndex--;
                it.value()->items.removeOne(url);
            }
            for (auto url : urls) {
                it.value()->items.insert(targetIndex++, url);
            }
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
            fmWarning() << "can not found :" << sourceId;
        }
        it = collections.find(targetKey);
        if (it != collections.end()) {
            for (auto url : urls) {
                it.value()->items.insert(targetIndex++, url);
            }
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

