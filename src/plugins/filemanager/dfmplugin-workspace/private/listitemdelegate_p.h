/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef LISTITEMDELEGATE_P_H
#define LISTITEMDELEGATE_P_H

#include "dfm_base_global.h"
#include "baseitemdelegate_p.h"
#include "listitemdelegate.h"

#include <QObject>

class ListItemDelegate;
class ListItemDelegatePrivate : public BaseItemDelegatePrivate
{
public:
    explicit ListItemDelegatePrivate(ListItemDelegate *qq);
    virtual ~ListItemDelegatePrivate();

    Q_DECLARE_PUBLIC(ListItemDelegate)
};

#endif   // LISTITEMDELEGATE_P_H
