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

#include "canvasviewbroker.h"
#include "canvasmanager.h"
#include "view/canvasview.h"
#include "view/canvasview_p.h"
#include "delegate/canvasitemdelegate_p.h"

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QRect *)
Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace ddplugin_canvas;

#define CanvasViewSlot(topic, args...) \
            dpfSlotChannel->connect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasViewDisconnect(topic) \
            dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic))

CanvasViewBroker::CanvasViewBroker(CanvasManager *mrg, QObject *parent)
    : QObject(parent)
    , manager(mrg)
{

}

CanvasViewBroker::~CanvasViewBroker()
{
    CanvasViewDisconnect(slot_CanvasView_VisualRect);
    CanvasViewDisconnect(slot_CanvasView_GridPos);
    CanvasViewDisconnect(slot_CanvasView_Refresh);
    CanvasViewDisconnect(slot_CanvasView_Update);
    CanvasViewDisconnect(slot_CanvasView_Select);
    CanvasViewDisconnect(slot_CanvasView_SelectedUrls);

    CanvasViewDisconnect(slot_CanvasItemDelegate_IconRect);
}

bool CanvasViewBroker::init()
{
    CanvasViewSlot(slot_CanvasView_VisualRect, &CanvasViewBroker::visualRect);
    CanvasViewSlot(slot_CanvasView_GridPos, &CanvasViewBroker::gridPos);
    CanvasViewSlot(slot_CanvasView_Refresh, &CanvasViewBroker::refresh);
    CanvasViewSlot(slot_CanvasView_Update, &CanvasViewBroker::update);
    CanvasViewSlot(slot_CanvasView_Select, &CanvasViewBroker::select);
    CanvasViewSlot(slot_CanvasView_SelectedUrls, &CanvasViewBroker::selectedUrls);

    CanvasViewSlot(slot_CanvasItemDelegate_IconRect, &CanvasViewBroker::iconRect);
    return true;
}

QSharedPointer<CanvasView> CanvasViewBroker::getView(int idx)
{
    // screen num is start with 1
    for (auto v : manager->views())
        if (v->screenNum() == idx)
            return v;

    return nullptr;
}

QRect CanvasViewBroker::visualRect(int idx, const QUrl &url)
{
    QRect rect;
    if (auto view = getView(idx))
        rect = view->d->visualRect(url.toString());
    return rect;
}

QPoint CanvasViewBroker::gridPos(int idx, const QPoint &viewPoint)
{
    QPoint pos;
    if (auto view = getView(idx))
        pos = view->d->gridAt(viewPoint);
    return pos;
}

void CanvasViewBroker::refresh(int idx)
{
    if (auto view = getView(idx))
        view->refresh();
}

void CanvasViewBroker::update(int idx)
{
    if (idx < 0) {
        for (auto view : manager->views())
            view->update();
        return;
    }

    if (auto view = getView(idx))
        view->update();
}

void CanvasViewBroker::select(const QList<QUrl> &urls)
{
    QItemSelection selection;
    auto m = manager->model();
    for (const QUrl &url: urls) {
        auto index = m->index(url);
        if (index.isValid())
            selection.append(QItemSelectionRange(index));
    }

    manager->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

QList<QUrl> CanvasViewBroker::selectedUrls(int idx)
{
    QList<QUrl> urls;
    auto files = manager->selectionModel()->selectedUrls();
    if (idx < 0) {
        urls = files;
    } else if (auto view = getView(idx)) {
        const int num = view->screenNum();
        QStringList items;
        items << GridIns->points(num).keys();
        items << GridIns->overloadItems(num);

        QList<QUrl> viewOn;
        for (const QUrl &url: files) {
            if (items.contains(url.toString()))
                viewOn.append(url);
        }
        urls = viewOn;
    }
    return urls;
}

QRect CanvasViewBroker::iconRect(int idx, QRect visualRect)
{
    QRect ret;
    if (auto view = getView(idx)) {
        visualRect = visualRect.marginsRemoved(view->d->gridMargins);
        ret = view->itemDelegate()->iconRect(visualRect);
    }
    return ret;
}

