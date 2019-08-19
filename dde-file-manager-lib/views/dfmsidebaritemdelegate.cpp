/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmsidebaritemdelegate.h"

#include "interfaces/dfmleftsidebaritem.h"

#include <QPainter>
#include <QDebug>

DFMSideBarItemDelegate::DFMSideBarItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{

}

void DFMSideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMLeftSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMLeftSideBarItem::Separator) {
        return paintSeparator(painter, option);
    }

    return QStyledItemDelegate::paint(painter, option, index);
}

QSize DFMSideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMLeftSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMLeftSideBarItem::Separator) {
        return sizeHintForType(DFMLeftSideBarItem::Separator);
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

void DFMSideBarItemDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    QColor bColor("red");

    painter->save();
    painter->setPen(bColor);
    int yPoint = option.rect.top() + option.rect.height() / 2;
    painter->drawLine(0, yPoint, option.rect.width(), yPoint);
    painter->restore();
}

QSize DFMSideBarItemDelegate::sizeHintForType(int type) const
{
    switch (type) {
    case DFMLeftSideBarItem::Separator:
        return QSize(200, 5);
    }

    return QSize(0, 0);
}
