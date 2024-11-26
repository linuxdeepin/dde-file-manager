// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKHANDLER_H
#define TASKHANDLER_H

#include "service_textindex_global.h"

#include <QString>
#include <functional>
#include <atomic>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// 定义任务处理器的回调类型，增加一个原子布尔引用参数用于控制运行状态
using TaskHandler = std::function<bool(const QString &path, std::atomic_bool &running)>;

// 工厂函数，返回具体的任务处理器
namespace TaskHandlers {
TaskHandler CreateIndexHandler();
TaskHandler UpdateIndexHandler();
}

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKHANDLER_H
