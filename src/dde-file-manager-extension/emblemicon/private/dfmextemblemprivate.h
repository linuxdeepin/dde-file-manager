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
#ifndef DFMEXTEMBLEMPRIVATE_H
#define DFMEXTEMBLEMPRIVATE_H

#include "dfm-extension-global.h"
#include "emblemicon/dfmextemblem.h"

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblem;
class DFMExtEmblemPrivate
{
    friend class DFMExtEmblem;

public:
    explicit DFMExtEmblemPrivate(DFMExtEmblem *qq);
    virtual ~DFMExtEmblemPrivate();
    void setEmblem(const std::vector<DFMExtEmblemIconLayout> &iconPath);
    std::vector<DFMExtEmblemIconLayout> emblems() const;

private:
    DFMExtEmblem *const q;
    std::vector<DFMExtEmblemIconLayout> emblemContainer {};
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTEMBLEMPRIVATE_H
