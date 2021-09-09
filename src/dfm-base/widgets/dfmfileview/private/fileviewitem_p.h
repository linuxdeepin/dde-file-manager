/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef FILEVIEWITEM_P_H
#define FILEVIEWITEM_P_H

#include "dfm-base/widgets/dfmfileview/fileviewitem.h"

class FileViewItem;
class FileViewItemPrivate
{
    Q_DECLARE_PUBLIC(FileViewItem)
    FileViewItem * const q_ptr;

    AbstractFileInfoPointer fileinfo;
    QMimeType mimeType;

public:
    explicit FileViewItemPrivate(FileViewItem* qq)
        : q_ptr(qq)
    {

    }
};

#endif // FILEVIEWITEM_P_H
