// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSORTFILTER_H
#define ABSTRACTSORTFILTER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QDirIterator>

namespace dfmbase {
class AbstractSortFilter
{
public:
    enum class SortScenarios : u_int8_t {
        kSortScenariosIteratorAddFile = 1,   // 迭代时，迭代出来的新文件是否进行排序
        kSortScenariosIteratorExistingFile = 2,   // 迭代时,迭代出来加入到了显示model中的文件
        kSortScenariosNormal = 3,   // 普通显示完成了的情况
        kSortScenariosWatcherAddFile = 4,   // 监视器监视到文件的添加 （包含重命名的添加）
        kSortScenariosWatcherOther = 5,   // 其他场景
    };

public:
    AbstractSortFilter();
    virtual ~AbstractSortFilter() {}
    // 左边比右边小返回true
    virtual int lessThan(const FileInfoPointer &left, const FileInfoPointer &right,
                         const bool isMixDirAndFile,
                         const Global::ItemRoles role,
                         const SortScenarios ss);
    virtual int checkFilters(const FileInfoPointer &info, const QDir::Filters filter, const QVariant &custum);
};
}

typedef QSharedPointer<DFMBASE_NAMESPACE::AbstractSortFilter> AbstractSortFilterPointer;

#endif   // ABSTRACTSORTFILTER_H
