// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICUCOLLATIONSTRATEGY_H
#define ICUCOLLATIONSTRATEGY_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/collation/collationstrategy.h>

#include <unicode/ucol.h>

DFMBASE_BEGIN_NAMESPACE

/**
 * @class IcuCollationStrategy
 * @brief 基于 ICU 的排序策略，直接产出 QByteArray 排序键。
 *
 * 按系统 locale 打开 UCollator，忠实复刻 QCollator（numericMode=true、
 * caseSensitivity=CaseSensitive）的 4 项 ICU 属性，使默认路径（不清空重排）
 * 与原 QCollator 行为逐字一致。当 @p clearReorder 为 true 时，额外调用
 * ucol_setReorderCodes(NULL, 0) 清空脚本重排——在简体中文（zh_CN）下，
 * CLDR 拼音排序默认含 [reorder Hani]，清空后拉丁字母排到中文之前，各脚本块
 * 内部顺序（含拼音序）保持不变。
 *
 * 排序键直接返回 QByteArray（ucol_getSortKey），避免 QCollatorSortKey 的共享
 * 指针间接与 std::variant 包装开销，使排序元素更紧凑、比较更快。
 *
 * 仅由 CollationStrategyProvider 创建：
 * - 默认（开关关 / 非 zh_CN）：clearReorder=false，行为等同原 QCollator。
 * - 开关开且 zh_CN：clearReorder=true，拉丁在前。
 */
class IcuCollationStrategy : public CollationStrategy
{
public:
    /**
     * @param clearReorder 是否清空脚本重排（拉丁在前）。默认 false。
     */
    explicit IcuCollationStrategy(bool clearReorder = false);
    ~IcuCollationStrategy() override;

    IcuCollationStrategy(const IcuCollationStrategy &) = delete;
    IcuCollationStrategy &operator=(const IcuCollationStrategy &) = delete;

    QByteArray sortKey(const QString &s) const override;
    int compare(const QString &s1, const QString &s2) const override;

private:
    /// ICU 排序器句柄；为空时走 UTF-8 字节序兜底（C locale 或 ucol_open 失败）
    UCollator *m_collator { nullptr };
    bool m_byteFallback { false };
};

DFMBASE_END_NAMESPACE

#endif   // ICUCOLLATIONSTRATEGY_H
