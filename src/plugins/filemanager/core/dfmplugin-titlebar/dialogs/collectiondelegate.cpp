// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectiondelegate.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

#include <DListView>
#include <dtkwidget_global.h>

DGUI_USE_NAMESPACE

CollectionDelegate::CollectionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
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
    QColor color;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        color.setRgb(0, 0, 0);
    else
        color.setRgb(240, 240, 255);
    if (index.row() % 2 == 0) {
        painter->setRenderHints(QPainter::Antialiasing
                                | QPainter::TextAntialiasing
                                | QPainter::SmoothPixmapTransform);
        color.setAlphaF(0.05);
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);
    }
    QStyledItemDelegate::paint(painter, option, index);
    if (option.state.testFlag(QStyle::State_Selected)) {
        QRect deleteButton(option.rect.width() - 30, option.rect.topRight().y() + 6, 24, 24);
        QPixmap px = QIcon(":icons/deepin/builtin/icons/dfm_close_round_normal_24px.svg").pixmap(QSize(24, 24));
        painter->drawPixmap(deleteButton, px);
    } else if (option.state.testFlag(QStyle::State_MouseOver)) {
        painter->setRenderHints(QPainter::Antialiasing
                                | QPainter::TextAntialiasing
                                | QPainter::SmoothPixmapTransform);
        color.setAlphaF(0.1);
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);

        QRect deleteButton(option.rect.width() - 30, option.rect.topRight().y() + 6, 24, 24);
        QPixmap px = QIcon(":icons/deepin/builtin/icons/dfm_close_round_normal_24px.svg").pixmap(QSize(24, 24));
        painter->drawPixmap(deleteButton, px);
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
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QVariant CollectionModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QString text = QStringListModel::data(index, role).toString();
        QUrl url(text);
        QString displayText = QString("%1://%2").arg(url.scheme()).arg(url.host());
        int port = url.port();
        if (port != -1)
            displayText += QString(":%1").arg(port);
        return displayText;
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
