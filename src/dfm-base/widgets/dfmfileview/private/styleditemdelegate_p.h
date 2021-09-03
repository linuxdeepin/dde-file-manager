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
#ifndef STYLEDITEMDELEGATE_P_H
#define STYLEDITEMDELEGATE_P_H

#include "widgets/dfmfileview/styleditemdelegate.h"

DFMBASE_BEGIN_NAMESPACE
class StyledItemDelegatePrivate
{
public:
    explicit StyledItemDelegatePrivate(StyledItemDelegate *qq)
        : q_ptr(qq) {}

    void init();
    void _q_onRowsInserted(const QModelIndex &parent, int first, int last);
    void _q_onRowsRemoved(const QModelIndex &parent, int first, int last);

    StyledItemDelegate *q_ptr;
    mutable QModelIndex editingIndex;
    QSize itemSizeHint;
    int textLineHeight = -1;

    Q_DECLARE_PUBLIC(StyledItemDelegate)
};
DFMBASE_END_NAMESPACE

#endif // STYLEDITEMDELEGATE_P_H
