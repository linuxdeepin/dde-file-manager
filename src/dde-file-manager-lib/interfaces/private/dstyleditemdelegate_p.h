// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSTYLEDITEMDELEGATE_P_H
#define DSTYLEDITEMDELEGATE_P_H

#include "dfmstyleditemdelegate.h"

class DFMStyledItemDelegatePrivate
{
public:
    explicit DFMStyledItemDelegatePrivate(DFMStyledItemDelegate *qq)
        : q_ptr(qq) {}

    void init();
    void _q_onRowsInserted(const QModelIndex &parent, int first, int last);
    void _q_onRowsRemoved(const QModelIndex &parent, int first, int last);

    DFMStyledItemDelegate *q_ptr;
    mutable QModelIndex editingIndex;
    QSize itemSizeHint;
    int textLineHeight = -1;

    Q_DECLARE_PUBLIC(DFMStyledItemDelegate)
};

#endif // DSTYLEDITEMDELEGATE_P_H
