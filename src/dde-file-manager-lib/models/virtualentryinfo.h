// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
