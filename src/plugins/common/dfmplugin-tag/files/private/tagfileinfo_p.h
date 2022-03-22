/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef TAGFILEINFO_P_H
#define TAGFILEINFO_P_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

DPTAG_BEGIN_NAMESPACE

class TagFileInfoPrivate : public DFMBASE_NAMESPACE::AbstractFileInfoPrivate
{
public:
    explicit TagFileInfoPrivate(DFMBASE_NAMESPACE::AbstractFileInfo *qq);

    virtual ~TagFileInfoPrivate();
};

DPTAG_END_NAMESPACE

#endif   // TAGFILEINFO_P_H
