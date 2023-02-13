// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-extension/emblemicon/dfmextemblem.h>
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
