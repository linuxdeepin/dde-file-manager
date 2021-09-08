/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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
#ifndef FILEVIEW_P_H
#define FILEVIEW_P_H

#include "dfm-base/dfm_base_global.h"
#include "widgets/dfmfileview/fileview.h"
#include "widgets/dfmfileview/fileviewmodel.h"
#include "widgets/dfmfileview/iconitemdelegate.h"
#include "widgets/dfmfileview/listitemdelegate.h"
#include "widgets/dfmfileview/headerview.h"

#include <QObject>
#include <QUrl>

namespace GlobalPrivate {

const int ICON_VIEW_SPACING = 5;
const int LIST_VIEW_SPACING = 1;
const int LIST_VIEW_MINIMUM_WIDTH = 80;
const int LIST_VIEW_ICON_SIZE = 24;

} //namespace GlobalPrivate

DFMBASE_BEGIN_NAMESPACE
class FileViewPrivate
{
    Q_DECLARE_PUBLIC(FileView)
    FileView * q_ptr;

    QAtomicInteger<bool> allowedAdjustColumnSize = true;
    HeaderView* headview = nullptr;
    ListItemDelegate *listDelegate = nullptr;
    IconItemDelegate *iconDelegate = nullptr;
    QUrl url;

    explicit FileViewPrivate(FileView *qq);
    int iconModeColumnCount(int itemWidth = 0) const;
};
DFMBASE_END_NAMESPACE

#endif // FILEVIEW_P_H
