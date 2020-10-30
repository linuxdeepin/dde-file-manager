/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
