// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXTRAVERSEUTILS_H
#define INDEXTRAVERSEUTILS_H

#include "service_textindex_global.h"

#include <QString>
#include <QSet>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexTraverseUtils {

// 检查是否是隐藏文件
bool isHiddenFile(const char *name);

// 检查是否是特殊目录(. 或 ..)
bool isSpecialDir(const char *name);

// 检查文件路径是否有效
bool isValidFile(const QString &path);

// 检查目录是否有效且未被访问过
bool isValidDirectory(const QString &path, QSet<QString> &visitedDirs);

// 检查是否应该跳过该目录
bool shouldSkipDirectory(const QString &path);

// 获取缓存的支持的文件扩展名集合
const QSet<QString> &getSupportedFileExtensions();

// 快速检查文件扩展名是否被支持（不创建QFileInfo对象）
bool isSupportedFileExtension(const QString &fileName);

// 从文件名中提取扩展名（不创建QFileInfo对象）
QString extractFileExtension(const QString &fileName);

QMap<QString, QString> fstabBindInfo();

}   // namespace IndexTraverseUtils

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXTRAVERSEUTILS_H
