// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountfileinfo.h"


MountFileInfo::MountFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{

}

MountFileInfo::~MountFileInfo()
{

}

bool MountFileInfo::canFetch() const
{
    return true;
}
