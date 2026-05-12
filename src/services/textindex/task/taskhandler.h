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
    bool useAnything { false };
    bool fatal { false };
    bool indexChanged { false };
};

using TaskHandler = std::function<HandlerResult(const QString &path, TaskState &state)>;

// 工厂函数，返回具体的任务处理器
namespace TaskHandlers {
TaskHandler CreateIndexHandler(const IndexContext &context);
TaskHandler UpdateIndexHandler(const IndexContext &context);

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
