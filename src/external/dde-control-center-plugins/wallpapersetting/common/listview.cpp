// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listview.h"
#include "itemdelegate.h"
#include "itemmodel.h"

#include <QDebug>
#include <QEvent>
#include <QHoverEvent>

#define ITEM_KEY "item"

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

ListView::ListView(QWidget *parent) : QListView(parent)
{

}

void ListView::initialize()
{
    setSelectionBehavior(SelectItems);
    setSelectionMode(NoSelection);

    setDragEnabled(false);
    setDragDropMode(NoDragDrop);
    setFrameShape(NoFrame);
    setSpacing(0);
    setViewportMargins(18, 10, 18, 10);

    delegate = new ItemDelegate(this);
    setItemDelegate(delegate);
    setViewMode(IconMode);
    setMovement(Static);

    setGridSize(delegate->sizeHint(QStyleOptionViewItem(), QModelIndex()));
    setResizeMode(Adjust);
    setFlow(LeftToRight);

    deleteBtn = new DIconButton(DStyle::SP_CloseButton, viewport());
    deleteBtn->setFixedSize(16, 16);
    deleteBtn->setIconSize(QSize(16, 16));
    deleteBtn->setFlat(true);
    deleteBtn->setFocusPolicy(Qt::NoFocus);
    deleteBtn->hide();

    connect(deleteBtn, &DIconButton::clicked, this, &ListView::onButtonClicked);
    connect(this, &ListView::clicked, this, &ListView::onItemClicked);
    connect(model(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &ListView::onItemRemoved);
}

ItemModel *ListView::itemModel() const
{
    return qobject_cast<ItemModel *>(model());
}

void ListView::setShowDelete(bool enable)
{
    enableDelete = enable;
    if (!enable) {
        deleteBtn->hide();
        deleteBtn->setProperty(ITEM_KEY, QString());
    }
}

void ListView::updateButton(const QModelIndex &index)
{
    auto item = itemModel()->itemNode(index);
    if (item && item->deletable && !selectionModel()->isSelected(index)) {
        deleteBtn->setProperty(ITEM_KEY, item->item);
        QPoint pos = visualRect(index).topRight() + QPoint(-16, 0);
        deleteBtn->move(pos);
        deleteBtn->show();
    } else {
        deleteBtn->hide();
        deleteBtn->setProperty(ITEM_KEY, QString());
    }
}

bool ListView::viewportEvent(QEvent *event)
{
    if (enableDelete) {
        switch (event->type()) {
        case QEvent::HoverMove:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::Leave:
        {
//            // use cursor pos instead of event->pos() to resolve
//            // that accepting Leave event when hover on deleteBtn
            auto pos = viewport()->mapFromGlobal(QCursor::pos());
            //auto pos = dynamic_cast<QHoverEvent *>(event)->pos();
            auto idx = indexAt(pos);
            updateButton(idx);
        }
            break;
        default:
            break;
        }
    }

    return QListView::viewportEvent(event);
}

void ListView::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (index.flags() & Qt::ItemIsSelectable) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
    }

    auto ptr = itemModel()->itemNode(index);
    if (!ptr.isNull())
        emit itemClicked(ptr);
}

void ListView::onButtonClicked()
{
    auto key = deleteBtn->property(ITEM_KEY).toString();
    qInfo() << "delete btn clicked" << key;

    auto m = itemModel();
    if (auto ptr = m->itemNode(m->itemIndex(key)))
        emit deleteButtonClicked(ptr);
}

void ListView::onItemRemoved(const QModelIndex &parent, int first, int last)
{
    auto key = deleteBtn->property(ITEM_KEY).toString();
    auto idx = itemModel()->itemIndex(key);
    if (idx.isValid() && idx.row() == first) {
          deleteBtn->hide();
          deleteBtn->setProperty(ITEM_KEY, QString());
     }
}
