// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void NormalizedModeBroker::refreshModel(bool global, int ms, bool file)
{
    if (auto m = mode->getModel())
        m->refresh(m->rootIndex(), global, ms, file);
}

QString NormalizedModeBroker::gridPoint(const QUrl &item, QPoint *point)
{
    CollectionViewBroker broker;
    for (auto holder : mode->d->holders.values()) {
        if (auto viewPtr = holder->widget()->view()) {
            broker.setView(viewPtr);
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

bool NormalizedModeBroker::selectAllItems()
{
    // select all colletions
    auto model = mode->getModel();
    auto selModel = mode->d->selectionModel;
    if (!model || !selModel)
        return false;

    selModel->selectAll();

    // and there is no item on canvas with normal mode,
    // so nothing to select on canvas.
    return true;
}
