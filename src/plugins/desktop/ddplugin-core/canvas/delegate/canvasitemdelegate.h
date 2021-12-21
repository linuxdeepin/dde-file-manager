/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef CANVASITEMDELEGATE_H
#define CANVASITEMDELEGATE_H

#include "dfm_desktop_service_global.h"

#include <QStyledItemDelegate>
#include <QTextOption>
#include <QTextLayout>

DSB_D_BEGIN_NAMESPACE

class CanvasView;
class CanvasItemDelegatePrivate;
class CanvasItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class CanvasItemDelegatePrivate;
public:
    explicit CanvasItemDelegate(QAbstractItemView *parentPtr = nullptr);
    ~CanvasItemDelegate() override;

    CanvasView *parent() const;
    QSize iconSize(int lv) const;
    int iconLevel() const;
    int setIconLevel(int lv);
    int minimumIconLevel() const;
    int maximumIconLevel() const;
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;
    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool mayExpand(QModelIndex *who = nullptr) const;
    static QRectF boundingRect(const QList<QRectF> &rects);
protected:
    virtual void initTextLayout(const QModelIndex &index, QTextLayout *layout) const;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QRect iconRect(const QRect &paintRect) const;
    static QRect labelRect(const QRect &paintRect, const QRect &usedRect);
    QRect textPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &label) const;
    static void paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                              QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    void paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const;
    void drawNormlText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const;
    void drawHighlightText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const;
    void drawExpandText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const;

    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                          QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);
    QList<QRectF> elideTextRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rect) const;
public:
    void updateItemSizeHint() const;
public:
    static const int kTextPadding;
    static const int kIconSpacing;
    static const int kTconBackRadius;
    static const int kIconRectRadius;
private:
    CanvasItemDelegatePrivate *const d = nullptr;

    //TODO(LQ):  QMutex mutex;
};

DSB_D_END_NAMESPACE
#endif   // CANVASITEMDELEGATE_H
