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

DDP_ORGANIZER_BEGIN_NAMESPACE

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
protected:
    virtual QString replace(const QUrl &oldUrl, const QUrl &newUrl) = 0;
    virtual QString append(const QUrl &) = 0;
    virtual QString remove(const QUrl &) = 0;
    virtual QString change(const QUrl &) = 0;
signals:
    // todo(wcl) add signals
protected:
    QHash<QString, CollectionBaseDataPtr> collections;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONDATAPROVIDER_H
