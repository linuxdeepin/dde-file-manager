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
#include "collectionholder_p.h"
#include "view/collectionview.h"
#include "models/fileproxymodel.h"

DDP_ORGANIZER_USE_NAMESPACE

CollectionHolderPrivate::CollectionHolderPrivate(CollectionHolder *qq) : q(qq)
{

}

CollectionHolder::CollectionHolder(const QString &uuid, QObject *parent)
    : QObject(parent)
    , d(new CollectionHolderPrivate(this))
{
    d->id = uuid;
}

QString CollectionHolder::id() const
{
    return d->id;
}

QString CollectionHolder::name()
{
    return d->name;
}

void CollectionHolder::setName(const QString &text)
{
    if (!text.isEmpty())
        d->name = text;

    // todo update title bar
}

QList<QUrl> CollectionHolder::urls() const
{
    return d->view->urls();
}

void CollectionHolder::setUrls(const QList<QUrl> &urls)
{
    d->view->setUrls(urls);
}

void CollectionHolder::createView(FileProxyModel *model)
{
    d->model = model;
    d->view = new CollectionView();
    d->view->setModel(d->model);
    d->view->setGeometry(QRect(200, 200, 400, 240));
    d->view->show();
}
