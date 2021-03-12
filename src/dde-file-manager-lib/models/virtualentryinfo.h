/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef VIRTUALENTRYINFO_H
#define VIRTUALENTRYINFO_H

#include "dabstractfileinfo.h"
#include "durl.h"

class VirtualEntryInfo : public DAbstractFileInfo
{
public:
    explicit VirtualEntryInfo(const DUrl &url);

    QString iconName() const override;

    bool exists() const override;

    bool isDir() const override;

    bool isVirtualEntry() const override;

    bool canShare() const override;

    bool isReadable() const override;

    bool isWritable() const override;

    QString fileName() const override;

    Qt::ItemFlags fileItemDisableFlags() const override;

    DAbstractFileInfo::CompareFunction compareFunByColumn(int) const override;
};

#endif // VIRTUALENTRYINFO_H
