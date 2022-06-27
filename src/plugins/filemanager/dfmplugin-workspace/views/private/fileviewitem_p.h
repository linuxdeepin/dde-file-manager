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
#ifndef FILEVIEWITEM_P_H
#define FILEVIEWITEM_P_H

#include "views/fileviewitem.h"

namespace dfmplugin_workspace {

class FileViewItem;
class FileViewItemPrivate : public QSharedData
{
    friend class FileViewItem;
    FileViewItem *const q;
    QMimeType mimeType;
    AbstractFileInfoPointer fileinfo;

public:
    explicit FileViewItemPrivate(FileViewItem *qq)
        : q(qq)
    {
    }
};

}

#endif   // FILEVIEWITEM_P_H
