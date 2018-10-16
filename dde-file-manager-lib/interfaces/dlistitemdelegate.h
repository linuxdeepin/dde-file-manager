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

#include "dstyleditemdelegate.h"

class DListItemDelegatePrivate;
class DListItemDelegate : public DStyledItemDelegate
{
public:
    explicit DListItemDelegate(DFileViewHelper *parent);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    void drawNotStringData(const QStyleOptionViewItem &opt, int lineHeight, const QRect &rect, const QVariant &data,
                           bool drawBackground, QPainter *painter, const int &column) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const Q_DECL_OVERRIDE;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget * editor, const QModelIndex & index) const Q_DECL_OVERRIDE;

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const Q_DECL_OVERRIDE;

    void updateItemSizeHint() Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    Q_DECLARE_PRIVATE(DListItemDelegate)
};

#endif // DLISTITEMDELEGATE_H
