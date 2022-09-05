// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDIRITERATOR_H
#define DDIRITERATOR_H

#include "dabstractfileinfo.h"

class DDirIterator
{
public:
    enum {
        // 把目录中的文件按文件系统的inode排序, 用于目录复制/移动
        // 在机械硬盘上, 读取排序后的文件内容会减少很多磁盘寻道和磁头定位的时间
        SortINode = 0x800
    };

    virtual ~DDirIterator() {}

    virtual DUrl next() = 0;
    virtual bool hasNext() const = 0;
    virtual void close() {}

    virtual QString fileName() const = 0;
    // Returns the full file url for the current directory entry.
    virtual DUrl fileUrl() const = 0;
    virtual const DAbstractFileInfoPointer fileInfo() const = 0;
    // Returns the base url of the iterator.
    virtual DUrl url() const = 0;

    virtual bool enableIteratorByKeyword(const QString &keyword) {Q_UNUSED(keyword); return false;}
public:
};

typedef QSharedPointer<DDirIterator> DDirIteratorPointer;

Q_DECLARE_METATYPE(DDirIteratorPointer)
#endif // DDIRITERATOR_H
