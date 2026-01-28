// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECLEANUPMANAGER_H
#define FILECLEANUPMANAGER_H

#include "dfmplugin_fileoperations_global.h"
#include <QUrl>
#include <QList>

DPFILEOPERATIONS_BEGIN_NAMESPACE

/**
 * 管理操作过程中创建但未完成的文件清理
 *
 * 职责：追踪不完整文件 → 在操作取消时删除
 */
class FileCleanupManager
{
public:
    FileCleanupManager() = default;
    ~FileCleanupManager() = default;

    /**
     * 追踪新创建的文件
     * 在文件创建时调用（文件已在磁盘存在但可能不完整）
     */
    void trackIncompleteFile(const QUrl &url);

    /**
     * 确认文件已完成，从追踪列表移除
     */
    void confirmCompleted(const QUrl &url);

    /**
     * 删除所有未完成的文件
     */
    void cleanupIncompleteFiles();

    /**
     * 检查是否有不完整文件
     */
    bool hasIncompleteFiles() const { return !incompleteFiles.isEmpty(); }

    /**
     * 获取未完成文件列表（用于调试）
     */
    QList<QUrl> getIncompleteFiles() const { return incompleteFiles; }

    /**
     * 清空列表
     */
    void clear() { incompleteFiles.clear(); }

private:
    QList<QUrl> incompleteFiles;
};

DPFILEOPERATIONS_END_NAMESPACE

#endif  // FILECLEANUPMANAGER_H
