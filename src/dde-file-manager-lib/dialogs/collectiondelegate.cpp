// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectiondelegate.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

CollectionDelegate::CollectionDelegate(QObject *parent):QStyledItemDelegate(parent)
{

}

CollectionDelegate::~CollectionDelegate()
{

}

QSize CollectionDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(376,36);
}

void CollectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);
    if(option.state.testFlag(QStyle::State_Selected)){
        QRect deleteButton(option.rect.width() - 30 ,option.rect.topRight().y()+6,24,24);
        QPixmap px = QIcon(":icons/deepin/builtin/icons/dfm_close_round_normal_24px.svg").pixmap(QSize(24,24));
        painter->drawPixmap(deleteButton,px);
    }else if(option.state.testFlag(QStyle::State_MouseOver)){
        painter->setRenderHints(QPainter::Antialiasing
                                |QPainter::TextAntialiasing
                                |QPainter::SmoothPixmapTransform);
        QColor color(0,0,0);
        color.setAlphaF(0.05);
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);

        QRect deleteButton(option.rect.width() - 30 ,option.rect.topRight().y()+6,24,24);
        QPixmap px = QIcon(":icons/deepin/builtin/icons/dfm_close_round_normal_24px.svg").pixmap(QSize(24,24));
        painter->drawPixmap(deleteButton,px);
    }
}


bool CollectionDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            QRect deleteButton(option.rect.width() - 30,option.rect.topRight().y()+6,24,24);
            QRect buttonRect(deleteButton.topLeft(), QSize(22,22));
            QPoint pos = e->pos();
            if (e->button()==Qt::LeftButton && buttonRect.contains(pos)) {
                emit removeItemManually(index.data().toString(),index.row());
                event->accept();
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event,model,option,index);
}
