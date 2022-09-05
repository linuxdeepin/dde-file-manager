// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    void paintFileName(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const int &role, const QRect &rect, const int &textLineHeight) const;
};

#endif // DLISTITEMDELEGATE_H
