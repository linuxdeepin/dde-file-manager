// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "highlightprovider.h"

#include <QMutexLocker>
#include <QDebug>

namespace dfmbase {

static const QString kPendingToken = QStringLiteral("__pending__");

HighlightProvider::HighlightProvider(QObject *parent)
    : QObject(parent)
{
    workerThread = new QThread(this);
    worker = new QObject();
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    workerThread->start();
}

HighlightProvider::~HighlightProvider()
{
    workerThread->quit();
    workerThread->wait(5000);
}

HighlightProvider *HighlightProvider::instance()
{
    static HighlightProvider ins;
    return &ins;
}

void HighlightProvider::setFetchCallback(FetchHighlightCallback cb)
{
    fetchCallback = std::move(cb);
}

void HighlightProvider::requestHighlight(const QString &taskId, const QString &path,
                                          const QString &keyword, int searchType,
                                          bool highPriority)
{
    if (!fetchCallback) {
        qWarning() << "HighlightProvider: fetchCallback not set, cannot request highlight";
        return;
    }

    // Step 1: 检查缓存
    {
        QMutexLocker lk(&cacheMutex);
        auto &taskCache = cache[taskId];
        auto it = taskCache.find(path);

        if (it != taskCache.end()) {
            const QString &cached = it.value();
            if (cached != kPendingToken) {
                // 已获取（包括空结果），直接返回
                lk.unlock();
                Q_EMIT highlightReady(taskId, path, cached);
                return;
            }
            // 标记为 pending，请求正在进行中，不重复提交
            return;
        }

        // 标记为 pending
        taskCache[path] = kPendingToken;
    }

    // Step 2: 加入请求队列
    {
        QMutexLocker lk(&requestMutex);

        if (highPriority) {
            pendingRequests.prepend({taskId, path, keyword, searchType});
        } else {
            pendingRequests.append({taskId, path, keyword, searchType});
        }

        // 如果工作线程空闲，触发处理
        int expected = 0;
        if (processing.load(std::memory_order_relaxed) == 0
            && processing.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            QMetaObject::invokeMethod(worker, [this]() {
                processNextRequest();
            }, Qt::QueuedConnection);
        }
    }
}

void HighlightProvider::cancelTask(const QString &taskId)
{
    // 从队列中移除所有该 taskId 的请求
    {
        QMutexLocker lk(&requestMutex);
        pendingRequests.erase(
            std::remove_if(pendingRequests.begin(), pendingRequests.end(),
                           [&taskId](const HighlightRequest &req) {
                               return req.taskId == taskId;
                           }),
            pendingRequests.end());
    }

    // 清除缓存
    {
        QMutexLocker lk(&cacheMutex);
        cache.remove(taskId);
    }
}

void HighlightProvider::processNextRequest()
{
    while (true) {
        HighlightRequest req;

        {
            QMutexLocker lk(&requestMutex);
            if (pendingRequests.isEmpty()) {
                processing.store(0, std::memory_order_release);
                // 双重检查：防止在 isEmpty 检查与 store 之间新增了请求
                if (!pendingRequests.isEmpty()) {
                    int expected = 0;
                    if (processing.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
                        continue;   // 继续处理新请求
                    }
                }
                return;   // 队列为空，退出
            }
            req = pendingRequests.takeFirst();
        }

        // 检查该 taskId 是否已被取消
        {
            QMutexLocker lk(&cacheMutex);
            if (!cache.contains(req.taskId)) {
                // 会话已被取消，跳过此请求
                continue;
            }
        }

        // 执行实际的 highlight 获取（同步阻塞 I/O，在工作线程中执行）
        QString content;
        if (fetchCallback) {
            content = fetchCallback(req.path, req.keyword, req.searchType);
        }

        // 存储到缓存（如果会话在 fetch 期间被取消，则跳过）
        {
            QMutexLocker lk(&cacheMutex);
            if (!cache.contains(req.taskId)) {
                // 会话在 fetch 期间被 cancelTask 取消，跳过存储和通知
                continue;
            }
            cache[req.taskId][req.path] = content;
        }

        // 通知主线程（跨线程信号自动 QueuedConnection）
        Q_EMIT highlightReady(req.taskId, req.path, content);
    }
}

}   // namespace dfmbase