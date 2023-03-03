// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTEMBLEMICONLAYOUTPRIVATE_H
#define DFMEXTEMBLEMICONLAYOUTPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/emblemicon/dfmextemblemiconlayout.h>

#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtEmblemIconLayoutPrivate
{
public:
    DFMExtEmblemIconLayoutPrivate(DFMExtEmblemIconLayout::LocationType type,
                                  const std::string &iconPath,
                                  int x = 0, int y = 0)
        : curType(type), curIconPath(iconPath), xPos(x), yPos(y)
    {
    }
    DFMExtEmblemIconLayout::LocationType curType { DFMExtEmblemIconLayout::LocationType::BottomRight };
    std::string curIconPath { "" };
    int xPos { 0 };
    int yPos { 0 };
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTEMBLEMICONLAYOUTPRIVATE_H
