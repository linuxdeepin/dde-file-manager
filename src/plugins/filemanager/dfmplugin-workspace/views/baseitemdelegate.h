/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include "dfmplugin_workspace_global.h"

#include <QStyledItemDelegate>
#include <QTextOption>

QT_BEGIN_NAMESPACE
class QTextLayout;
QT_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE

class FileViewHelper;
class FileView;
class BaseItemDelegatePrivate;
class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseItemDelegate(FileViewHelper *parent);

    virtual ~BaseItemDelegate() override;

    /*!
     * \brief paintGeomertys paint geomertys for all items
     * \param option
     * \param index
     * \param sizeHintMode
     * \return
     */
    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const = 0;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

    /**
     * @brief iconSizeLevel return icon size level
     * @return Return current icon level if level is vaild, otherwise return -1
     */
    virtual int iconSizeLevel() const;

    /**
     * @brief minimumIconSizeLevel return minimum icon size level
     * @return Return minimum icon level if level is vaild, otherwise return -1
     */
    virtual int minimumIconSizeLevel() const;

    /**
     * @brief maximumIconSizeLevel return maximum icon size level
     * @return Return maximum icon level if level is vaild, otherwise return -1
     */
    virtual int maximumIconSizeLevel() const;

    /**
     * @brief increaseIcon increase icon size level
     * @return  Return current icon level if icon can increase; otherwise return -1.
     */
    virtual int increaseIcon();

    /**
     * @brief increaseIcon decrease icon size level
     * @return  Return current icon level if icon can decrease; otherwise return -1.
     */
    virtual int decreaseIcon();

    /*!
     * \brief setIconSizeByIconSizeLevel set icon size level
     * \param level
     * \return  Return current icon level if level is vaild, otherwise return -1
     */
    virtual int setIconSizeByIconSizeLevel(int level);

    /**
     * @brief hasWidgetIndexs
     * @return
     */
    virtual QModelIndexList hasWidgetIndexs() const;

    /**
     * @brief hideAllIIndexWidget
     */
    virtual void hideAllIIndexWidget();
    /**
     * @brief hideNotEditingIndexWidget
     */
    virtual void hideNotEditingIndexWidget();

    /**
     * @brief commitDataAndCloseActiveEditor
     */
    virtual void commitDataAndCloseActiveEditor();

    /**
     * @brief updateItemSizeHint when icon size change or font change on pain event call
     */
    virtual void updateItemSizeHint() = 0;

    virtual QList<QRectF> drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout,
                                   const QRectF &boundingRect, qreal radius, const QBrush &background,
                                   QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere,
                                   Qt::TextElideMode mode = Qt::ElideMiddle, int flags = Qt::AlignCenter,
                                   const QColor &shadowColor = QColor()) const;

    QList<QRectF> drawText(const QModelIndex &index, QPainter *painter, const QString &text,
                           const QRectF &boundingRect, qreal radius, const QBrush &background,
                           QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere,
                           Qt::TextElideMode mode = Qt::ElideMiddle, int flags = Qt::AlignCenter,
                           const QColor &shadowColor = QColor()) const;

    QModelIndex editingIndex() const;

    QWidget *editingIndexWidget() const;

    FileViewHelper *parent() const;

    void paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const;
    QSize getIndexIconSize(const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const;

protected:
    explicit BaseItemDelegate(BaseItemDelegatePrivate &dd, FileViewHelper *parent);

    virtual void initTextLayout(const QModelIndex &index, QTextLayout *layout) const;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    QList<QRectF> getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const;
    QScopedPointer<BaseItemDelegatePrivate> d;
    Q_DECLARE_PRIVATE_D(d, BaseItemDelegate)
};

DPWORKSPACE_END_NAMESPACE

#endif   // BASEITEMDELEGATE_H
