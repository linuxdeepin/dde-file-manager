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

DDP_ORGANIZER_USE_NAMESPACE

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
