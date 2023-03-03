// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARITEMDELEGATE_H
#define SIDEBARITEMDELEGATE_H

#include "dfmplugin_sidebar_global.h"

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE
DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SideBarItemDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
Q_SIGNALS:
    void rename(const QModelIndex &index, QString newName) const;
};
DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARITEMDELEGATE_H
