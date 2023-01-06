// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dfmglobal.h"
#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DFMSideBarItemDelegate(QAbstractItemView *parent = nullptr);

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
signals:
    void rename(const QModelIndex &index, QString newName) const;
private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
    QSize sizeHintForType(int type) const;
};

DFM_END_NAMESPACE
