// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUNTFILEINFO_H
#define MOUNTFILEINFO_H

#include "dfileinfo.h"

class MountFileInfo : public DAbstractFileInfo
{
public:
    explicit MountFileInfo(const DUrl &url);
    ~MountFileInfo();

    bool canFetch() const override;
};

#endif // MOUNTFILEINFO_H
