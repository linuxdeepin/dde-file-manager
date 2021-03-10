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
