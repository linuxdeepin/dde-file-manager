/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include "dfm-base/dfm_base_global.h"

#include <QStyledItemDelegate>
#include <QTextOption>
#include <QObject>

#include <DListView>
DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QTextLayout;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class StyledItemDelegatePrivate;
class StyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit StyledItemDelegate(DListView *parent);
    ~StyledItemDelegate() override;

    DListView *parent() const;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
    QString displayText(const QVariant &value, const QLocale &locale) const override;

    virtual QModelIndexList hasWidgetIndexs() const;
    virtual void hideAllIIndexWidget();
    virtual void hideNotEditingIndexWidget();
    virtual void commitDataAndCloseActiveEditor();
    virtual QRect fileNameRect(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const = 0;

    virtual int iconSizeLevel() const;
    virtual int minimumIconSizeLevel() const;
    virtual int maximumIconSizeLevel() const;

    virtual int increaseIcon();
    virtual int decreaseIcon();
    virtual int setIconSizeByIconSizeLevel(int level);

    virtual void updateItemSizeHint() = 0;

    static void paintCircleList(QPainter *painter, QRectF boundingRect, qreal diameter, const QList<QColor> &colors, const QColor &borderColor);
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

protected:
    StyledItemDelegate(StyledItemDelegatePrivate &dd, DListView *parent);

    virtual void initTextLayout(const QModelIndex &index, QTextLayout *layout) const;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QList<QRectF> getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const;

    static void paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment = Qt::AlignCenter,
                          QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    QScopedPointer<StyledItemDelegatePrivate> d_ptr;

    Q_PRIVATE_SLOT(d_ptr, void _q_onRowsInserted(const QModelIndex &parent, int first, int last))
    Q_PRIVATE_SLOT(d_ptr, void _q_onRowsRemoved(const QModelIndex &parent, int first, int last))

    Q_DECLARE_PRIVATE(StyledItemDelegate)
    Q_DISABLE_COPY(StyledItemDelegate)
};
DFMBASE_END_NAMESPACE

#endif // STYLEDITEMDELEGATE_H
