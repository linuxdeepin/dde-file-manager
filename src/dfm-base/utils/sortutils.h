// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTUTILS_H
#define SORTUTILS_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/sortfileinfo.h>

DFMGLOBAL_BEGIN_NAMESPACE

namespace SortUtils {
bool compareStringDefault(const QString &str1, const QString &str2);
bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order);
bool compareStringForFileName(const QString &str1, const QString &str2);
bool compareStringForTime(const QString &str1, const QString &str2);
bool compareStringForMimeType(const QString &str1, const QString &str2);
bool compareForSize(const SortInfoPointer info1, const SortInfoPointer info2);

bool isNumOrChar(const QChar ch);
bool isNumber(const QChar ch);
bool isSymbol(const QChar ch);
QString numberStr(const QString &str, int pos);
bool isFullWidthChar(const QChar ch, QChar &normalized);
QString makeQString(const QString::const_iterator &it, uint unicode);
qint64 getEffectiveSize(const SortInfoPointer &info);
}   // namespace SortUtils

DFMGLOBAL_END_NAMESPACE

#endif   // SORTUTILS_H
