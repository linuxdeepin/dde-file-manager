/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
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
#ifndef DEFAULTCANVASITEMDELEGATE_H
#define DEFAULTCANVASITEMDELEGATE_H

#include "dfm-base/widgets/abstractcanvasdelegate.h"

#include <QTextOption>

class QTextLayout;
class DefaultCanvasView;
class DefaultCanvasItemDelegatePrivate;
class DefaultCanvasItemDelegate : public dfmbase::AbstractCanvasDelegate
{
    Q_OBJECT
    friend class DefaultCanvasItemDelegatePrivate;

public:
    explicit DefaultCanvasItemDelegate(dfmbase::AbstractCanvas *parent = nullptr);
    ~DefaultCanvasItemDelegate() override;

    int currentIconSizeLevel() const;
    QSize getIconSizeByIconSizeLevel(const int lv) const;
    QSize getCurrentIconSize() const;
    int setIconSizeByIconSizeLevel(const int lv);
    int minimumIconSizeLevel() const;
    int maximumIconSizeLevel() const;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void updateItemSizeHint();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    virtual QList<QRectF> drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout, const QRectF &boundingRect, qreal radius, const QBrush &background, QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::TextElideMode mode = Qt::ElideMiddle, int flags = Qt::AlignCenter, const QColor &shadowColor = QColor()) const;
    QList<QRectF> drawText(const QModelIndex &index, QPainter *painter, const QString &text,
                           const QRectF &boundingRect, qreal radius, const QBrush &background,
                           QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere,
                           Qt::TextElideMode mode = Qt::ElideMiddle, int flags = Qt::AlignCenter,
                           const QColor &shadowColor = QColor()) const;
    DefaultCanvasView *parent() const;

    QModelIndexList hasWidgetIndexs() const;
    void hideNotEditingIndexWidget();
    QModelIndex expandedIndex() const;
    QWidget *expandedIndexWidget() const;

private:
    bool isTransparent(const QModelIndex &index) const;
    static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    QList<QRectF> getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const;

    QRectF initIconGeometry(const QStyleOptionViewItem &opt) const;
    void initTextLayout(const QModelIndex &index, QTextLayout *layout) const;
    QColor itemBaseColor(const QStyleOptionViewItem &opt, bool isSelected, bool isDropTarget = false) const;
    void setIconBaseParameter(QPainter *painter, const QColor &clr, const QStyleOptionViewItem &option, bool isSelected, bool isDragMode = false) const;
    QRectF setTextBaseParameter(QPainter *painter, const QStyleOptionViewItem &opt, const QRectF &iconRect, bool isSelected, bool isDragMode = false) const;
    void drawIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                  QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    void drawFileName(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index, const QString &str, const QRectF labelRect) const;

private:
    mutable QModelIndex lastAndExpandedIndex;
    QSize itemSizeHint;
    DefaultCanvasItemDelegatePrivate *const d = nullptr;

    //    QMutex mutex; //暂时不见得需要
};

#endif   // DEFAULTCANVASITEMDELEGATE_H
