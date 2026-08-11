// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icucollationstrategy.h"

#include <QLocale>
#include <QByteArray>

DFMBASE_BEGIN_NAMESPACE

IcuCollationStrategy::IcuCollationStrategy(bool clearReorder)
{
    const QLocale loc = QLocale::system();

    // C locale：QCollator 退化为字节序，此处同步回退
    if (loc.language() == QLocale::C) {
        m_byteFallback = true;
        return;
    }

    UErrorCode err = U_ZERO_ERROR;
    m_collator = ucol_open(loc.name().toUtf8().constData(), &err);
    if (U_FAILURE(err) || !m_collator) {
        m_byteFallback = true;
        return;
    }

    // 忠实复刻 QCollator(numericMode=true, caseSensitivity=CaseSensitive) 设置的
    // 4 项 ICU 属性（见 qtbase qcollator_icu.cpp QCollatorPrivate::init）。只设这
    // 4 项、不额外设 FRENCH_COLLATION/CASE_FIRST/CASE_LEVEL，与 QCollator 完全对齐，
    // 保证默认路径（不清空重排）与原 QCollator 排序结果逐字一致。
    // UCOL_DEFAULT_STRENGTH 让 ICU 按 locale 解析（多数=TERTIARY，日文=QUATERNARY），
    // 与 QCollator 大小写敏感分支一致；勿硬编码 TERTIARY 以免 ja 等语系偏差。
    // 每次调用前重置 err：ICU 在 err 已失败时后续调用会静默 no-op。
    auto setAttr = [&](UColAttribute a, UColAttributeValue v) {
        err = U_ZERO_ERROR;
        ucol_setAttribute(m_collator, a, v, &err);
        if (U_FAILURE(err)) {
            m_byteFallback = true;
            ucol_close(m_collator);
            m_collator = nullptr;
            return false;
        }
        return true;
    };
    if (!setAttr(UCOL_NORMALIZATION_MODE, UCOL_ON)) return;
    if (!setAttr(UCOL_STRENGTH, UCOL_DEFAULT_STRENGTH)) return;
    if (!setAttr(UCOL_NUMERIC_COLLATION, UCOL_ON)) return;
    if (!setAttr(UCOL_ALTERNATE_HANDLING, UCOL_NON_IGNORABLE)) return;

    // 可选：清空脚本重排，使拉丁字母排到中文之前。对无重排的 locale 为 no-op。
    if (clearReorder) {
        err = U_ZERO_ERROR;
        ucol_setReorderCodes(m_collator, nullptr, 0, &err);
        if (U_FAILURE(err)) {
            m_byteFallback = true;
            ucol_close(m_collator);
            m_collator = nullptr;
        }
    }
}

IcuCollationStrategy::~IcuCollationStrategy()
{
    if (m_collator)
        ucol_close(m_collator);
}

QByteArray IcuCollationStrategy::sortKey(const QString &s) const
{
    if (m_byteFallback || !m_collator)
        return s.toUtf8();

    const UChar *utf16 = reinterpret_cast<const UChar *>(s.utf16());
    const int32_t len = s.size();

    // 启发式缓冲区生成；不够则按返回值扩容重试。ucol_getSortKey 不接收 UErrorCode。
    QByteArray key;
    key.resize(len * 4 + 16);
    int32_t n = ucol_getSortKey(m_collator, utf16, len,
                                reinterpret_cast<uint8_t *>(key.data()), key.size());
    if (n > key.size()) {
        key.resize(n);
        n = ucol_getSortKey(m_collator, utf16, len,
                            reinterpret_cast<uint8_t *>(key.data()), n);
    }
    key.resize(n > 0 ? n : 0);
    return key;
}

int IcuCollationStrategy::compare(const QString &s1, const QString &s2) const
{
    if (m_byteFallback || !m_collator)
        return s1.toUtf8().compare(s2.toUtf8());

    // 直接走 UTF-16 路径（ucol_strcoll），与 sortKey() 的 utf16() 路径对称。
    return ucol_strcoll(m_collator,
                        reinterpret_cast<const UChar *>(s1.utf16()), s1.size(),
                        reinterpret_cast<const UChar *>(s2.utf16()), s2.size());
}

DFMBASE_END_NAMESPACE
