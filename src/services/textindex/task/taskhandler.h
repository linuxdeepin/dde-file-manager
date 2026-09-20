// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKHANDLER_H
#define TASKHANDLER_H

#include "service_textindex_global.h"
#include "core/indexcontext.h"
#include "fileprovider.h"

#include "utils/taskstate.h"

#include <QString>
#include <functional>
#include <QMetaType>
#include <memory>

SERVICETEXTINDEX_BEGIN_NAMESPACE

struct HandlerResult
{
    bool success { false };
    bool interrupted { false };
    bool paused { false };
    bool useAnything { false };
    bool fatal { false };
    bool indexChanged { false };
    QStringList remainingFiles;
};

using TaskHandler = std::function<HandlerResult(const QString &path, TaskState &state)>;

// 工厂函数，返回具体的任务处理器
namespace TaskHandlers {
TaskHandler CreateIndexHandler(const IndexContext &context);

/// @param skipStaleCleanup 内部恢复类 Update（Dirty/断连恢复、needsRebuild 静默更新）
///     传 true：跳过开头的 cleanupIndexs 全库清理（数百万条目时为分钟级 I/O）。
///     删除本应由增量事件维护，cleanup 只是事件丢失/黑名单变更的兜底；跳过后
///     丢失窗口内被删文件会在索引中残留 ghost 条目，由下一次用户手动
///     "更新索引"（skipStaleCleanup=false）清理。用户经 DBus 触发的更新保持默认 false。
TaskHandler UpdateIndexHandler(const IndexContext &context, bool skipStaleCleanup = false);

/// Handler for resuming an interrupted Create task. Skips cleanupIndexs, uses cached
/// file list + checkpoint from IndexStateStore when available, or falls back to BFS.
/// Only selected by TaskManager when createInProgress is true.
TaskHandler CreateResumeHandler(const IndexContext &context);

// 文件列表任务处理器
TaskHandler CreateOrUpdateFileListHandler(const IndexContext &context, const QStringList &fileList);
TaskHandler RemoveFileListHandler(const IndexContext &context, const QStringList &fileList);   // 待实现的删除索引接口

// 创建文件提供者
std::unique_ptr<FileProvider> createFileProvider(const IndexContext &context, const QString &path);
std::unique_ptr<FileProvider> createFileListProvider(const IndexContext &context, const QStringList &fileList);
TaskHandler MoveFileListHandler(const IndexContext &context, const QHash<QString, QString> &movedFiles);
}

SERVICETEXTINDEX_END_NAMESPACE

Q_DECLARE_METATYPE(SERVICETEXTINDEX_NAMESPACE::HandlerResult)

#endif   // TASKHANDLER_H
