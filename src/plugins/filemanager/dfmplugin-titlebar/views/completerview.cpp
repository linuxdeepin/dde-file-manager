/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "views/private/completerview_p.h"
#include "views/completerview.h"

#include <QScrollBar>

DPTITLEBAR_USE_NAMESPACE
CompleterView::CompleterView(QWidget *parent)
    : QListView(parent), d(new CompleterViewPrivate(this))
{
    overrideWindowFlags(Qt::Tool /*| Qt::WindowDoesNotAcceptFocus*/);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportMargins(0, 0, -verticalScrollBar()->sizeHint().width(), 0);
    setMouseTracking(true);

    setItemDelegate(&d->delegate);
    //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复，设置这个属性listview就可以拥有地址兰的输入法
    setAttribute(Qt::WA_InputMethodEnabled);
}

void CompleterView::keyPressEvent(QKeyEvent *e)
{
    return QListView::keyPressEvent(e);
}

void CompleterView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    emit listCurrentChanged(current);
    QListView::currentChanged(current, previous);
}

void CompleterView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    emit listSelectionChanged(selected);
    QListView::selectionChanged(selected, deselected);
}

CompleterViewDelegate::CompleterViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CompleterViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // prepare
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
            ? QPalette::Normal
            : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // draw background
    if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw text
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->setFont(option.font);
    painter->drawText(option.rect.adjusted(31, 0, 0, 0), Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
}

QSize CompleterViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(24);

    return s;
}

CompleterViewPrivate::CompleterViewPrivate(CompleterView *qq)
    : QObject(qq), q(qq)
{
}
