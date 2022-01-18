/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef ICONITEMDELEGATE_P_H
#define ICONITEMDELEGATE_P_H

#include "dfm_base_global.h"
#include "baseitemdelegate_p.h"
#include "views/iconitemdelegate.h"
#include "views/expandedItem.h"

#include <QPointer>

DPWORKSPACE_BEGIN_NAMESPACE

class IconItemDelegate;
class IconItemDelegatePrivate : public BaseItemDelegatePrivate
{
public:
    explicit IconItemDelegatePrivate(IconItemDelegate *qq);
    ~IconItemDelegatePrivate();

    QIcon checkedIcon = QIcon::fromTheme("emblem-checked");
    QSize itemIconSize;

    QPointer<ExpandedItem> expandedItem;
    mutable QModelIndex lastAndExpandedInde;
    mutable QModelIndex expandedIndex;

    // default icon size is 64px.
    int currentIconSizeIndex { 1 };

    QTextDocument *document { nullptr };
    Q_DECLARE_PUBLIC(IconItemDelegate)
};

DPWORKSPACE_END_NAMESPACE

#endif   // ICONITEMDELEGATE_P_H
