/*
* Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "dfmextemblem.h"
#include "private/dfmextemblemprivate.h"

#include <assert.h>

USING_DFMEXT_NAMESPACE

void DFMExtEmblem::setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPaths)
{
    d->setEmblem(iconPaths);
}

std::vector<DFMExtEmblemIconLayout> DFMExtEmblem::emblems() const
{
    return d->emblems();
}

DFMExtEmblem::DFMExtEmblem()
    : d(new DFMExtEmblemPrivate(this))
{
}

DFMExtEmblem::~DFMExtEmblem()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

DFMExtEmblem::DFMExtEmblem(const DFMExtEmblem &emblem)
{
    if (d) {
        delete d;
        d = nullptr;
    }
    d = new DFMExtEmblemPrivate(this);
    setEmblem(emblem.emblems());
}

DFMExtEmblem &DFMExtEmblem::operator=(const DFMExtEmblem &emblem)
{
    if (this == &emblem)
        return *this;
    if (d) {
        delete d;
        d = nullptr;
    }
    d = new DFMExtEmblemPrivate(this);
    setEmblem(emblem.emblems());
    return *this;
}
