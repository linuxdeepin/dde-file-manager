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

#include "interfaces/dfmsidebaritem.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DPalette>
#include <qdrawutil.h>
#include <QLineEdit>

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMSideBarItemDelegate::DFMSideBarItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

void DFMSideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMSideBarItem::Separator) {
        return paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DFMSideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant v = index.data(DFMSideBarItem::ItemTypeRole);
    if (v.isValid() && v.toInt() == DFMSideBarItem::Separator) {
        return sizeHintForType(DFMSideBarItem::Separator);
    } else {
        return DStyledItemDelegate::sizeHint(option, index);
    }
}

void DFMSideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(model);
    QByteArray n = editor->metaObject()->userProperty().name();
    if (!n.isEmpty()) {
        emit rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *DFMSideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        QRegExp regx("[^\\\\/:\\*\\?\"<>|%&]+");
        QValidator *validator = new QRegExpValidator(regx, qle);
        qle->setValidator(validator);
    }

    return editor;
}

void DFMSideBarItemDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}

QSize DFMSideBarItemDelegate::sizeHintForType(int type) const
{
    switch (type) {
    case DFMSideBarItem::Separator:
        return QSize(200, 5);
    }

    return QSize(0, 0);
}

DFM_END_NAMESPACE
