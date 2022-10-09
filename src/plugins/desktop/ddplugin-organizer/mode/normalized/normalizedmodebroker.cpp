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
#include "normalizedmodebroker.h"
#include "normalizedmode_p.h"
#include "view/collectionwidget.h"
#include "view/collectionview.h"
#include "broker/collectionviewbroker.h"
#include "delegate/collectionitemdelegate.h"
#include "models/collectionmodel.h"

using namespace ddplugin_organizer;

NormalizedModeBroker::NormalizedModeBroker(NormalizedMode *parent)
    : OrganizerBroker(parent)
    , mode(parent)
{
    Q_ASSERT(mode);
}

void NormalizedModeBroker::refreshModel(bool global, int ms)
{
    if (auto m = mode->getModel())
        m->refresh(m->rootIndex(), global, ms);
}

QString NormalizedModeBroker::gridPoint(const QUrl &item, QPoint *point)
{
    CollectionViewBroker broker;
    for (auto holder : mode->d->holders.values()) {
        if (auto view = holder->widget()->view()) {
            broker.setView(view);
            QPoint pos;
            if (broker.gridPoint(item, pos)) {
                if (point)
                    *point = pos;
                return holder->id();
            }
        }
    }
    return "";
}

QRect NormalizedModeBroker::visualRect(const QString &id, const QUrl &item)
{
    QRect ret;

    CollectionHolderPointer holder = mode->d->holders.value(id);
    if (!holder.isNull()) {
        CollectionViewBroker broker(holder->widget()->view());
        ret = broker.visualRect(item);
    }

    return ret;
}

QAbstractItemView *NormalizedModeBroker::view(const QString &id)
{
    QAbstractItemView *ret = nullptr;
    CollectionHolderPointer holder = mode->d->holders.value(id);
    if (!holder.isNull())
        ret = holder->widget()->view();

    return ret;
}

QRect NormalizedModeBroker::iconRect(const QString &id, QRect vrect)
{
    QRect ret;
    CollectionHolderPointer holder = mode->d->holders.value(id);
    if (!holder.isNull())
        if (auto delegage = holder->widget()->view()->itemDelegate())
            ret = delegage->iconRect(vrect);
    return ret;
}
