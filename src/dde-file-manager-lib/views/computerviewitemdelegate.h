// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERVIEWITEMDELEGATE_H
#define COMPUTERVIEWITEMDELEGATE_H

#include <QStyledItemDelegate>

#include "computerview.h"

class ComputerViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComputerViewItemDelegate(QObject *parent = nullptr);
    ~ComputerViewItemDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void closeEditingEditor(ComputerListView *view);

private:
    ComputerView *par;
    mutable QLineEdit *editingEditor {nullptr};
    mutable QModelIndex editingIndex;
};


#endif // COMPUTERVIEWITEMDELEGATE_H
