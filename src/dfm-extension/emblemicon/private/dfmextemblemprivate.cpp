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
#include "dfmextemblemprivate.h"

USING_DFMEXT_NAMESPACE

DFMExtEmblemPrivate::DFMExtEmblemPrivate(DFMExtEmblem *qq)
    : q(qq)
{

}

DFMExtEmblemPrivate::~DFMExtEmblemPrivate()
{

}

void DFMExtEmblemPrivate::setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPath)
{
    emblemContainer.clear();
    emblemContainer = iconPath;
}

std::vector<DFMExtEmblemIconLayout> DFMExtEmblemPrivate::emblems() const
{
    return emblemContainer;
}
