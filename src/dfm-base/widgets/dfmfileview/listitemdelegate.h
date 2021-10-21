/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef LISTITEMDELEGATE_H
#define LISTITEMDELEGATE_H

#include "dfm-base/dfm_base_global.h"

#include <DListView>

#include <QStyledItemDelegate>

DFMBASE_BEGIN_NAMESPACE
class ListItemDelegatePrivate;
class ListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class ListItemDelegatePrivate;
    ListItemDelegatePrivate *const d;
public:
    explicit ListItemDelegate(DTK_WIDGET_NAMESPACE::DListView *parent = nullptr);
    virtual ~ListItemDelegate() override;
    void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
private:
    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    QRect paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    void paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index, const QRect &iconRect) const;
};
DFMBASE_END_NAMESPACE

#endif // DLISTITEMDELEGATE_H
