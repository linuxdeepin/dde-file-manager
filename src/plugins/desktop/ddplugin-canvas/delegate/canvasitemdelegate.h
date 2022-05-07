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

#include "ddplugin_canvas_global.h"

#include <QStyledItemDelegate>
#include <QTextOption>
#include <QTextLayout>

DDP_CANVAS_BEGIN_NAMESPACE

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
    QString iconSizeLevelDescription(int i) const;
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;
    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    bool mayExpand(QModelIndex *who = nullptr) const;
    static QRectF boundingRect(const QList<QRectF> &rects);
    QSize paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QRect iconRect(const QRect &paintRect) const;
    static QRect labelRect(const QRect &paintRect, const QRect &usedRect);
    QRect textPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, bool elide) const;
    static QRect paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                           QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static QRectF paintEmblems(QPainter *painter, const QRectF &rect, const QUrl &url);
    static bool extendPaintText(QPainter *painter, const QUrl &url, QRectF *rect);
    void paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rLabel) const;
    void drawNormlText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rText) const;
    void drawHighlightText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText) const;
    void drawExpandText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const;

    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);
    QList<QRectF> elideTextRect(const QModelIndex &index, const QRect &rect, const Qt::TextElideMode &elideMode) const;
    bool isTransparent(const QModelIndex &index) const;
public slots:
    void updateItemSizeHint() const;
    void commitDataAndCloseEditor();
    void revertAndcloseEditor();
protected slots:
    void clipboardDataChanged();

public:
    static const int kTextPadding;
    static const int kIconSpacing;
    static const int kIconBackRadius;
    static const int kIconRectRadius;

private:
    CanvasItemDelegatePrivate *const d = nullptr;
};

DDP_CANVAS_END_NAMESPACE
#endif   // CANVASITEMDELEGATE_H
