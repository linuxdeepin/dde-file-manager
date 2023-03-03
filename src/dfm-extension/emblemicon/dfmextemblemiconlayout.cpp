// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/dfmextemblemiconlayoutprivate.h"

#include <dfm-extension/emblemicon/dfmextemblemiconlayout.h>

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
