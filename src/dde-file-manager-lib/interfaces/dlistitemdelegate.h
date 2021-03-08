/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DLISTITEMDELEGATE_H
#define DLISTITEMDELEGATE_H

#include "dfmstyleditemdelegate.h"

class DListItemDelegatePrivate;
class DListItemDelegate : public DFMStyledItemDelegate
{
public:
    explicit DListItemDelegate(DFileViewHelper *parent);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void drawNotStringData(const QStyleOptionViewItem &opt, int lineHeight, const QRect &rect, const QVariant &data,
                           bool drawBackground, QPainter *painter, const int &column) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override;

    void updateItemSizeHint() override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    Q_DECLARE_PRIVATE(DListItemDelegate)
};

#endif // DLISTITEMDELEGATE_H
