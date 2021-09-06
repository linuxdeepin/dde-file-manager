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
#ifndef ICONITEMDELEGATE_H
#define ICONITEMDELEGATE_H

#include "styleditemdelegate.h"
#include "dfm-base/dfm_base_global.h"

#include <QPointer>
#include <QMutex>

class FileIconItem;
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class IconItemDelegatePrivate;
class IconItemDelegate : public StyledItemDelegate
{
    Q_OBJECT

    Q_PROPERTY(QColor focusTextBackgroundBorderColor READ focusTextBackgroundBorderColor WRITE setFocusTextBackgroundBorderColor)
    Q_PROPERTY(bool enabledTextShadow READ enabledTextShadow WRITE setEnabledTextShadow)

public:
    explicit IconItemDelegate(DListView *parent);
    ~IconItemDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override;

    QModelIndexList hasWidgetIndexs() const override;
    void hideNotEditingIndexWidget() override;

    int iconSizeLevel() const override;
    int minimumIconSizeLevel() const override;
    int maximumIconSizeLevel() const override;

    int increaseIcon() override;
    int decreaseIcon() override;
    int setIconSizeByIconSizeLevel(int level) override;

    void updateItemSizeHint() override;

    QColor focusTextBackgroundBorderColor() const;
    bool enabledTextShadow() const;
    QSize iconSizeByIconSizeLevel() const;

public Q_SLOTS:
    void setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor);
    void setEnabledTextShadow(bool enabledTextShadow);

protected:
    void initTextLayout(const QModelIndex &index, QTextLayout *layout) const override;

    bool eventFilter(QObject *object, QEvent *event) override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void onEditWidgetFocusOut();
    void onTriggerEdit(const QModelIndex &index);
    QIcon m_checkedIcon;
    QMutex m_mutex; //析构和构造索
    QWindow* m_focusWindow = nullptr;

    Q_DECLARE_PRIVATE(IconItemDelegate)

    friend class ExpandedItem;
};
DFMBASE_END_NAMESPACE

#endif // DFMICONITEMDELEGATE_H
