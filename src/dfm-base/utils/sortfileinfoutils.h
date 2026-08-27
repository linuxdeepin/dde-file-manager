// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTFILEINFOUTILS_H
#define SORTFILEINFOUTILS_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <QSet>
#include <QString>

DFMBASE_BEGIN_NAMESPACE

namespace SortInfoUtils {

/**
 * @brief 加载 .hidden 文件中的隐藏文件列表
 * @param dirPath 目录路径
 * @return 隐藏文件名的集合
 */
QSet<QString> loadHideFileList(const QString &dirPath);

/**
 * @brief 通过 statx 创建 SortFileInfo
 * @param parentPath  父目录路径
 * @param fileName    文件名
 * @param hideList    隐藏文件列表（.hidden 文件内容）
 * @return SortInfoPointer，失败返回 nullptr
 *
 * 使用 statx 获取文件的全部属性（含 birth time），
 * 自动处理符号链接的目标属性解析。
 */
SortInfoPointer createSortInfo(const QString &parentPath,
                               const QString &fileName,
                               const QSet<QString> &hideList);

}   // namespace SortInfoUtils

DFMBASE_END_NAMESPACE

#endif   // SORTFILEINFOUTILS_H
