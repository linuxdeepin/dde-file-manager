/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
