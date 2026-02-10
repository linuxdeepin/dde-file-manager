// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASITEMDELEGATE_H
#define CANVASITEMDELEGATE_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QStyledItemDelegate>
#include <QTextOption>
#include <QTextLayout>

namespace ddplugin_canvas {

class CanvasView;
class CanvasViewBroker;
class CanvasItemDelegatePrivate;
class CanvasItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class CanvasItemDelegatePrivate;
    friend class CanvasViewBroker;

public:
    struct PaintIconOpts
    {
        QRectF rect;
        Qt::Alignment alignment { Qt::AlignCenter };
        QIcon::Mode mode { QIcon::Normal };
        QIcon::State state { QIcon::Off };
        bool isThumb { false };
    };

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
    QRect expendedGeomerty(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    bool mayExpand(QModelIndex *who = nullptr) const;
    static QRectF boundingRect(const QList<QRectF> &rects);
    QSize paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
    int textLineHeight() const;

public:
    QRect iconRect(const QRect &paintRect) const;
    static QRect labelRect(const QRect &paintRect, const QRect &usedRect);

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QRect textPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, bool elide) const;
    static std::optional<QRectF> paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts);
    static QRectF paintEmblems(QPainter *painter, const QRectF &rect, const FileInfoPointer &info);

    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &iconRect) const;
    void paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rLabel) const;
    void drawNormlText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rText) const;
    void drawHighlightText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText) const;
    void drawExpandText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const;

    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);
    static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);
    QList<QRectF> elideTextRect(const QModelIndex &index, const QRect &rect, const Qt::TextElideMode &elideMode) const;
    bool isTransparent(const QModelIndex &index) const;
    bool isThumnailIconIndex(const QModelIndex &index) const;

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
    static const int kIconBackgroundMargin;

private:
    CanvasItemDelegatePrivate *const d = nullptr;
};

}
#endif   // CANVASITEMDELEGATE_H
