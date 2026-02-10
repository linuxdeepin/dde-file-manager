// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTUTILS_H
#define SORTUTILS_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/sortfileinfo.h>

DFMBASE_BEGIN_NAMESPACE

namespace SortUtils {
bool compareString(const QString &str1, const QString &str2, Qt::SortOrder order);
bool compareStringForFileName(const QString &str1, const QString &str2);
bool compareStringForTime(const QString &str1, const QString &str2);
bool compareStringForMimeType(const QString &str1, const QString &str2);
bool compareForSize(const SortInfoPointer info1, const SortInfoPointer info2);
bool compareForSize(const qint64 size1, const qint64 size2);

QString accurateDisplayType(const QUrl &url);
QString accurateLocalMimeType(const QUrl &url);

// private
qint64 getEffectiveSize(const SortInfoPointer &info);
QString getLocalPath(const QUrl &url);
}   // namespace SortUtils

DFMBASE_END_NAMESPACE

#endif   // SORTUTILS_H
