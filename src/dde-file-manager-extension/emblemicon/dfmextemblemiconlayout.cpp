/*
* Copyright (C) 2021 Deepin Technology Co., Ltd.
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
#include "dfmextemblemiconlayout.h"
#include "private/dfmextemblemiconlayoutprivate.h"

USING_DFMEXT_NAMESPACE

DFMExtEmblemIconLayout::DFMExtEmblemIconLayout(LocationType type,
                                               const std::string &path,
                                               int x, int y)
    : d(new DFMExtEmblemIconLayoutPrivate(type, path, x, y))
{

}

DFMExtEmblemIconLayout::~DFMExtEmblemIconLayout()
{
    if (d) {
        delete d;
        d = nullptr;
    }

}

DFMExtEmblemIconLayout::DFMExtEmblemIconLayout(const DFMExtEmblemIconLayout &emblem)
{
    if (d) {
        delete d;
        d = nullptr;
    }
    d = new DFMExtEmblemIconLayoutPrivate(emblem.locationType(),
                                          emblem.iconPath(),
                                          emblem.x(), emblem.y());
}

DFMExtEmblemIconLayout &DFMExtEmblemIconLayout::operator=(const DFMExtEmblemIconLayout &emblem)
{
    if (this == &emblem)
        return *this;
    if (d) {
        delete d;
        d = nullptr;
    }
    d = new DFMExtEmblemIconLayoutPrivate(emblem.locationType(),
                                          emblem.iconPath(),
                                          emblem.x(), emblem.y());
    return *this;
}

int DFMExtEmblemIconLayout::x() const
{
    return d->xPos;
}

int DFMExtEmblemIconLayout::y() const
{
    return d->yPos;
}

DFMExtEmblemIconLayout::LocationType dfmext::DFMExtEmblemIconLayout::locationType() const
{
    return d->curType;
}

std::string DFMExtEmblemIconLayout::iconPath() const
{
    return d->curIconPath;
}
