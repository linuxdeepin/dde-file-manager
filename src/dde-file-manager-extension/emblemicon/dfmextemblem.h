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
#ifndef DFMEXTEMBLEM_H
#define DFMEXTEMBLEM_H

#include "dfm-extension-global.h"
#include "dfmextemblemiconlayout.h"

#include <vector>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemPrivate;
class DFMExtEmblem
{
    friend class DFMExtEmblemPrivate;
public:
    explicit DFMExtEmblem();
    ~DFMExtEmblem();
    DFMExtEmblem(const DFMExtEmblem &emblem);
    DFMExtEmblem &operator=(const DFMExtEmblem &emblem);
    void setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPaths);
    std::vector<DFMExtEmblemIconLayout> emblems() const;

private:
    DFMExtEmblemPrivate *d { nullptr };
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTEMBLEM_H
