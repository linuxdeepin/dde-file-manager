// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLATIONSTRATEGY_H
#define COLLATIONSTRATEGY_H

#include <dfm-base/dfm_base_global.h>

#include <QByteArray>
#include <QString>

DFMBASE_BEGIN_NAMESPACE

/**
 * @class CollationStrategy
 * @brief 排序策略抽象接口（Strategy 模式）。
 *
 * 将"按哪种排序规则生成排序键 / 比较字符串"抽象为接口，当前唯一实现为
 * IcuCollationStrategy（基于 ICU，按 clearReorder 决定是否清空脚本重排）。
 *
 * 排序键统一为 QByteArray（ucol_getSortKey 直出），避免 QCollatorSortKey 的
 * 共享指针间接与 std::variant 包装带来的排序热路径性能开销。
 *
 * 调用方（FileNameSorter）依赖此抽象而非具体后端（DIP / OCP）。
 */
class CollationStrategy
{
public:
    virtual ~CollationStrategy() = default;

    /// 生成排序键（用于批量预计算 + std::stable_sort）
    virtual QByteArray sortKey(const QString &s) const = 0;

    /// 直接比较两个字符串（用于逐对比较场景）
    /// @return <0 表示 s1 排在 s2 之前，0 表示相等，>0 表示 s1 排在 s2 之后
    virtual int compare(const QString &s1, const QString &s2) const = 0;
};

DFMBASE_END_NAMESPACE

#endif   // COLLATIONSTRATEGY_H
