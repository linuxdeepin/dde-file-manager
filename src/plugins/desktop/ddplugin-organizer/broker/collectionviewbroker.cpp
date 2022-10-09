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
#include "collectionviewbroker.h"
#include "view/collectionview_p.h"

#include "models/collectionmodel.h"

using namespace ddplugin_organizer;

CollectionViewBroker::CollectionViewBroker(CollectionView *parent)
    : QObject(parent)
    , view(parent)
{

}

void CollectionViewBroker::setView(CollectionView *v)
{
    view = v;
    setParent(v);
}

bool CollectionViewBroker::gridPoint(const QUrl &file, QPoint &pos) const
{
    auto items = view->d->provider->items(view->d->id);
    int node = items.indexOf(file);
    if (node >= 0) {
        pos = view->d->nodeToPos(node);
        return true;
    }

    return false;
}

QRect CollectionViewBroker::visualRect(const QUrl &file) const
{
    QRect rect;
    auto items = view->d->provider->items(view->d->id);
    int node = items.indexOf(file);
    if (node >= 0) {
        auto pos = view->d->nodeToPos(node);
        rect = view->d->visualRect(pos);
    }

    return rect;
}
