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

DPTITLEBAR_USE_NAMESPACE
CompleterView::CompleterView()
    : d(new CompleterViewPrivate(this))
{
    this->setModel(&d->model);

    d->completer.setModel(&d->model);
    d->completer.setModel(&d->model);
    d->completer.setPopup(this);
    d->completer.setCompletionMode(QCompleter::PopupCompletion);
    d->completer.setCaseSensitivity(Qt::CaseSensitive);
    d->completer.setMaxVisibleItems(10);

    QObject::connect(&d->completer, QOverload<const QString &>::of(&QCompleter::activated),
                     this, QOverload<const QString &>::of(&CompleterView::completerActivated));

    QObject::connect(&d->completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
                     this, QOverload<const QModelIndex &>::of(&CompleterView::completerActivated));

    QObject::connect(&d->completer, QOverload<const QString &>::of(&QCompleter::highlighted),
                     this, QOverload<const QString &>::of(&CompleterView::completerHighlighted));

    QObject::connect(&d->completer, QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
                     this, QOverload<const QModelIndex &>::of(&CompleterView::completerHighlighted));

    setItemDelegate(&d->delegate);
}

QCompleter *CompleterView::completer()
{
    return &d->completer;
}

QStringListModel *CompleterView::model()
{
    return &d->model;
}

CompleterViewDelegate *CompleterView::itemDelegate()
{
    return &d->delegate;
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
