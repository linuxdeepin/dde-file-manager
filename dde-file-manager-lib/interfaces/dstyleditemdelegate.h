/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DSTYLEDITEMDELEGATE_H
#define DSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class DFileViewHelper;
class DStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DStyledItemDelegate(DFileViewHelper *parent);

    DFileViewHelper *parent() const;

    QModelIndex editingIndex() const;
    QWidget *editingIndexWidget() const;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QString displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;

    virtual QModelIndexList hasWidgetIndexs() const;
    virtual void hideAllIIndexWidget();
    virtual void hideNotEditingIndexWidget();
    virtual void commitDataAndCloseActiveEditor();
    virtual QRect fileNameRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

    virtual int iconSizeLevel() const;
    virtual int minimumIconSizeLevel() const;
    virtual int maximumIconSizeLevel() const;

    virtual int increaseIcon();
    virtual int decreaseIcon();
    virtual int setIconSizeByIconSizeLevel(int level);

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QList<QRect> getCornerGeometryList(const QRect &baseRect, const QSize &cornerSize) const;

    mutable QModelIndex editing_index;
    QSize m_itemSizeHint;
};

#endif // DSTYLEDITEMDELEGATE_H
