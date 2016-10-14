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

#include "dstyleditemdelegate.h"

class DStyledItemDelegatePrivate
{
public:
    DStyledItemDelegatePrivate(DStyledItemDelegate *qq)
        : q_ptr(qq) {}

    DStyledItemDelegate *q_ptr;
    mutable QModelIndex editingIndex;
    QSize itemSizeHint;

    Q_DECLARE_PUBLIC(DStyledItemDelegate)
};

#endif // DSTYLEDITEMDELEGATE_P_H
