// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTUTILS_H
#define SORTUTILS_H

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/sortfileinfo.h>

DFMBASE_BEGIN_NAMESPACE

/**
 * @namespace SortUtils
 * @brief 排序工具函数
 *
 * @note 此命名空间中的函数主要用于 FileSortWorker。
 *       对于文件名排序，建议直接使用 FileNameSorter 类以获得更好的性能。
 *       这些函数内部委托给 FileNameSorter 实现。
 */
namespace SortUtils {

// 文件名比较 - 委托给 FileNameSorter
bool compareStringForFileName(const QString &str1, const QString &str2);

// 时间字符串比较（格式：YYYY/MM/DD hh:mm:ss）
bool compareStringForTime(const QString &str1, const QString &str2);

// MIME类型比较
bool compareStringForMimeType(const QString &str1, const QString &str2);

// 大小比较
bool compareForSize(const SortInfoPointer info1, const SortInfoPointer info2);
bool compareForSize(const qint64 size1, const qint64 size2);

// 工具函数
QString accurateDisplayType(const QUrl &url);
QString accurateLocalMimeType(const QUrl &url);

// private - 内部使用
qint64 getEffectiveSize(const SortInfoPointer &info);
QString getLocalPath(const QUrl &url);

}   // namespace SortUtils

DFMBASE_END_NAMESPACE

#endif   // SORTUTILS_H
