// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKHANDLER_H
#define TASKHANDLER_H

#include "service_textindex_global.h"

#include "utils/taskstate.h"

#include <QString>
#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

using TaskHandler = std::function<bool(const QString &path, TaskState &state)>;

// 工厂函数，返回具体的任务处理器
namespace TaskHandlers {
TaskHandler CreateIndexHandler();
TaskHandler UpdateIndexHandler();
TaskHandler RemoveIndexHandler();
}

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKHANDLER_H
