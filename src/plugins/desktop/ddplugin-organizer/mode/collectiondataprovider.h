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
#ifndef COLLECTIONDATAPROVIDER_H
#define COLLECTIONDATAPROVIDER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>
#include <QHash>

namespace ddplugin_organizer {

class CollectionDataProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CollectionDataProvider)
public:
    explicit CollectionDataProvider(QObject *parent = nullptr);
    virtual ~CollectionDataProvider();
    virtual QString key(const QUrl &) const;
    virtual QString name(const QString &key) const;
    virtual QList<QString> keys() const;
    virtual QList<QUrl> items(const QString &key) const;
    virtual void moveUrls(const QList<QUrl> &urls, const QString &targetKey, int targetIndex);
    virtual void addPreItems(const QString &targetKey, const QList<QUrl> &urls, int targetIndex);
    virtual bool checkPreItem(const QUrl &url, QString &key, int &index);
    virtual bool takePreItem(const QUrl &url, QString &key, int &index);
protected:
    virtual QString replace(const QUrl &oldUrl, const QUrl &newUrl) = 0;
    virtual QString append(const QUrl &) = 0;
    virtual void insert(const QUrl &, const QString &, const int) = 0;
    virtual QString remove(const QUrl &) = 0;
    virtual QString change(const QUrl &) = 0;
signals:
    void nameChanged(const QString &key, const QString &name);
    void itemsChanged(const QString &key);
protected:
    QHash<QString, CollectionBaseDataPtr> collections;
    QHash<QString, QPair<int, QList<QUrl>>> preCollectionItems;
};

}

#endif // COLLECTIONDATAPROVIDER_H
