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
#ifndef DFMEXTEMBLEMICONLAYOUTPRIVATE_H
#define DFMEXTEMBLEMICONLAYOUTPRIVATE_H

#include "dfm-extension-global.h"
#include "emblemicon/dfmextemblemiconlayout.h"

#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemIconLayoutPrivate {
public:
    DFMExtEmblemIconLayoutPrivate(DFMExtEmblemIconLayout::LocationType type,
                                  const std::string &iconPath,
                                  int x = 0, int y = 0)
        : curType(type)
        , curIconPath(iconPath)
        , xPos(x)
        , yPos(y)
    {
    }
    DFMExtEmblemIconLayout::LocationType curType { DFMExtEmblemIconLayout::LocationType::BottomRight };
    std::string curIconPath { "" };
    int xPos { 0 };
    int yPos { 0 };
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTEMBLEMICONLAYOUTPRIVATE_H
