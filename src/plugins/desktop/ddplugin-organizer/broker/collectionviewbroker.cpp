// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
