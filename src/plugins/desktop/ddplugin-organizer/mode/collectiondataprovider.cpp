/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
    for (const CollectionBaseDataPtr &ptr: collections.values()) {
        if (ptr->items.contains(url)) {
            ret = ptr->key;
            break;
        }
    }
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
                    qWarning() << "unknow error:" << url << it.value()->items;
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
            qWarning() << "can not found :" << sourceId;
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
        QPair<int, QList<QUrl>> items{targetIndex, urls};
        preCollectionItems.insert(targetKey, items);
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

