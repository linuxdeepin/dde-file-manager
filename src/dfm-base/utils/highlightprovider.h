// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIGHLIGHTPROVIDER_H
#define HIGHLIGHTPROVIDER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHash>
#include <QList>
#include <functional>
#include <atomic>

namespace dfmbase {

class HighlightProvider : public QObject
{
    Q_OBJECT

public:
    // 回调类型：path → keyword → searchType → highlightedContent
    using FetchHighlightCallback = std::function<QString(const QString &path,
                                                          const QString &keyword,
                                                          int searchType)>;

    static HighlightProvider *instance();

    // 注入实际的 fetchHighlight 实现（由 search 插件调用）
    void setFetchCallback(FetchHighlightCallback cb);

    // 请求单个文件的 highlightContent
    // highPriority: true 表示可见区域请求，插入队列头部优先处理
    void requestHighlight(const QString &taskId, const QString &path,
                          const QString &keyword, int searchType,
                          bool highPriority = false);

    // 取消指定搜索会话的所有待处理请求
    void cancelTask(const QString &taskId);

Q_SIGNALS:
    // highlightContent 获取完成信号
    void highlightReady(const QString &taskId, const QString &path, const QString &content);

private Q_SLOTS:
    void processNextRequest();

private:
    struct HighlightRequest
    {
        QString taskId;
        QString path;
        QString keyword;
        int searchType;
    };

    explicit HighlightProvider(QObject *parent = nullptr);
    ~HighlightProvider() override;

    FetchHighlightCallback fetchCallback;
    QThread *workerThread = nullptr;
    QObject *worker = nullptr;   // 工作线程上的占位 QObject（用于槽函数调度）

    mutable QMutex requestMutex;
    QList<HighlightRequest> pendingRequests;   // 优先级队列：高优先级插入头部
    std::atomic<int> processing { 0 };         // 工作线程处理状态标记

    // 缓存：QHash<taskId, QHash<path, content>>
    // 未请求 = 不存在于 map 中
    // 特殊 "__pending__" = 请求中
    // 空 QString = 已获取但无高亮内容
    // 非空内容 = 已获取且有高亮内容
    QHash<QString, QHash<QString, QString>> cache;
    QMutex cacheMutex;
};

}   // namespace dfmbase

#endif   // HIGHLIGHTPROVIDER_H
