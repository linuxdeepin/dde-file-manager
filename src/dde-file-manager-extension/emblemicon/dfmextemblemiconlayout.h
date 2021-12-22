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
#ifndef DFMEXTEMBLEMICONLAYOUT_H
#define DFMEXTEMBLEMICONLAYOUT_H

#include "dfm-extension-global.h"

#include <string>
#include <memory>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemIconLayoutPrivate;
class DFMExtEmblemIconLayout
{
    friend class DFMExtEmblemIconLayoutPrivate;

public:
    enum class LocationType : uint8_t
    {
        BottomRight = 0,
        BottomLeft,
        TopLeft,
        TopRight,
        Custom = 0xff
    };

    explicit DFMExtEmblemIconLayout(LocationType type,
                           const std::string &path,
                           int x = 0, int y = 0);
    ~DFMExtEmblemIconLayout();
    DFMExtEmblemIconLayout(const DFMExtEmblemIconLayout &emblem);
    DFMExtEmblemIconLayout &operator=(const DFMExtEmblemIconLayout &emblem);
    LocationType locationType() const;
    std::string iconPath() const;
    int x() const;
    int y() const;

private:
    DFMExtEmblemIconLayoutPrivate *d { nullptr };
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTEMBLEMICONLAYOUT_H
