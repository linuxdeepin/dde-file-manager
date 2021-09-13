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
#ifndef ICONITEMDELEGATE_H
#define ICONITEMDELEGATE_H

#include "dfm-base/dfm_base_global.h"

#include <DListView>

#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class FileIconItem;
class IconItemDelegatePrivate;
class IconItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class IconItemDelegatePrivate;
    IconItemDelegatePrivate *const d;
    friend class ExpandedItem;
public:
    explicit IconItemDelegate(DTK_WIDGET_NAMESPACE::DListView *parent);
    virtual ~IconItemDelegate() override;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};
DFMBASE_END_NAMESPACE

#endif // DFMICONITEMDELEGATE_H
