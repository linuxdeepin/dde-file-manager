/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHFILEINFO_P_H
#define SEARCHFILEINFO_P_H

#include "dfmplugin_search_global.h"

#include "interfaces/private/abstractfileinfo_p.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

class SearchFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit SearchFileInfoPrivate(AbstractFileInfo *qq);
    virtual ~SearchFileInfoPrivate();
    void setProxy(const AbstractFileInfoPointer &aproxy);

public:
    AbstractFileInfoPointer proxy;
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHFILEINFO_P_H
