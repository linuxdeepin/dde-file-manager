// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
