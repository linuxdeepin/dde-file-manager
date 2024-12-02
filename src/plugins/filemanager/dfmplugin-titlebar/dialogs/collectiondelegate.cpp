// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectiondelegate.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include <DListView>
#include <DPaletteHelper>
#include <dtkwidget_global.h>

CollectionDelegate::CollectionDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

CollectionDelegate::~CollectionDelegate()
{
}

QSize CollectionDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(376, 36);
}

void CollectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // use QStyle avoid backgrounds overwrited by DTK.
    QStyledItemDelegate::paint(painter, option, index);

    painter->setRenderHints(QPainter::Antialiasing
                            | QPainter::TextAntialiasing
                            | QPainter::SmoothPixmapTransform);

    {   // draw background.
        QColor color(240, 240, 255);
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
            color.setRgb(0, 0, 0);
        if (index.row() % 2 == 0) {
            color.setAlphaF(0.05);
            painter->setBrush(color);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(option.rect, 8, 8);
        }
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            color.setAlphaF(0.1);
            painter->setBrush(color);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(option.rect, 8, 8);
        }
    }

    {   // draw delete button.
        if (option.state.testFlag(QStyle::State_MouseOver)
            || option.state.testFlag(QStyle::State_Selected)) {
            QRect deleteButton(option.rect.width() - 30, option.rect.topRight().y() + 6, 24, 24);
            static const QPixmap px = QIcon::fromTheme("dfm_close_round_normal").pixmap(QSize(24, 24));
            painter->drawPixmap(deleteButton, px);
        }
    }
}

bool CollectionDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            QRect deleteButton(option.rect.width() - 30, option.rect.topRight().y() + 6, 24, 24);
            QRect buttonRect(deleteButton.topLeft(), QSize(22, 22));
            QPoint pos = e->pos();
            if (e->button() == Qt::LeftButton && buttonRect.contains(pos)) {
                emit removeItemManually(index.data().toString(), index.row());
                event->accept();
                return true;
            }
        }
    }
    return DStyledItemDelegate::editorEvent(event, model, option, index);
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QString text = QStringListModel::data(index, role).toString();
        return text;   // show full collection.
        // QUrl url(text);
        // QString displayText = QString("%1://%2").arg(url.scheme()).arg(url.host());
        // int port = url.port();
        // if (port != -1)
        //     displayText += QString(":%1").arg(port);
        // return displayText;
    } else if (role == kUrlRole) {
        return QStringListModel::data(index, Qt::DisplayRole);
    }
    return QStringListModel::data(index, role);
}

int CollectionModel::findItem(const QString &item)
{
    const QStringList &items = QStringListModel::stringList();
    return items.indexOf(item);
}
