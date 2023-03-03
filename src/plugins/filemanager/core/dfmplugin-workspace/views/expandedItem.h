// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPANDEDITEM_H
#define EXPANDEDITEM_H

#include "dfmplugin_workspace_global.h"

#include <QWidget>
#include <QStyleOptionViewItem>
namespace dfmplugin_workspace {

class IconItemDelegate;
class ExpandedItem : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ getOpacity WRITE setOpacity)

public:
    explicit ExpandedItem(IconItemDelegate *d, QWidget *parent = nullptr);
    ~ExpandedItem() override;

    bool event(QEvent *ee) override;
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;
    int heightForWidth(int width) const override;

    qreal getOpacity() const;
    void setOpacity(qreal opacity);
    void setIconPixmap(const QPixmap &pixmap, int height);
    QRectF getTextBounding() const;
    void setTextBounding(QRectF textBounding);
    int getIconHeight() const;
    void setIconHeight(int iconHeight);
    bool getCanDeferredDelete() const;
    void setCanDeferredDelete(bool canDeferredDelete);
    QModelIndex getIndex() const;
    void setIndex(QModelIndex index);
    QStyleOptionViewItem getOption() const;
    void setOption(QStyleOptionViewItem opt);
    QRectF textGeometry(int width = -1) const;
    QRectF iconGeometry() const;

private:
    QPixmap iconPixmap;
    int iconHeight { 0 };
    mutable QRectF textBounding;
    QModelIndex index;
    QStyleOptionViewItem option;
    qreal opacity { 1 };
    bool canDeferredDelete { true };
    IconItemDelegate *delegate { nullptr };
};
}

#endif   // EXPANDEDITEM_H
