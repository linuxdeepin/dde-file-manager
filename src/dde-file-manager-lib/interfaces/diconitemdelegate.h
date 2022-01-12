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

#ifndef DFILEITEMDELEGATE_H
#define DFILEITEMDELEGATE_H

#include "dfmstyleditemdelegate.h"

#include <QPointer>
#include <QMutex>

class FileIconItem;
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class DIconItemDelegatePrivate;
class DIconItemDelegate : public DFMStyledItemDelegate
{
    Q_OBJECT

    Q_PROPERTY(QColor focusTextBackgroundBorderColor READ focusTextBackgroundBorderColor WRITE setFocusTextBackgroundBorderColor)
    Q_PROPERTY(bool enabledTextShadow READ enabledTextShadow WRITE setEnabledTextShadow)

public:
    explicit DIconItemDelegate(DFileViewHelper *parent);
    ~DIconItemDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override;

    QModelIndexList hasWidgetIndexs() const override;
    void hideNotEditingIndexWidget() override;

    QModelIndex expandedIndex() const;
    QWidget *expandedIndexWidget() const;

    int iconSizeLevel() const override;
    int minimumIconSizeLevel() const override;
    int maximumIconSizeLevel() const override;

    int increaseIcon() override;
    int decreaseIcon() override;
    int setIconSizeByIconSizeLevel(int level) override;

    void updateItemSizeHint() override;

    QColor focusTextBackgroundBorderColor() const;
    bool enabledTextShadow() const;

public slots:
    void setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor);
    void setEnabledTextShadow(bool enabledTextShadow);

protected:
    void initTextLayout(const QModelIndex &index, QTextLayout *layout) const override;

    bool eventFilter(QObject *object, QEvent *event) override;

    QList<QRectF> drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout,
                           const QRectF &boundingRect, qreal radius, const QBrush &background,
                           QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere,
                           Qt::TextElideMode mode = Qt::ElideMiddle, int flags = Qt::AlignCenter,
                           const QColor &shadowColor = QColor()) const override;

    using DFMStyledItemDelegate::drawText;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void onEditWidgetFocusOut();
    void onTriggerEdit(const QModelIndex &index);
    QSize iconSizeByIconSizeLevel() const;
    QIcon m_checkedIcon;
    QMutex m_mutex; //析构和构造索
    QWindow* m_focusWindow = nullptr;

    Q_DECLARE_PRIVATE(DIconItemDelegate)

    friend class ExpandedItem;
};

#endif // DFILEITEMDELEGATE_H
