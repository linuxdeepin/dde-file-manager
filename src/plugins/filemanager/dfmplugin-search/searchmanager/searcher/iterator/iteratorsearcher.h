// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include "dfm-base/dfm_base_global.h"
#include "searchmanager/searcher/abstractsearcher.h"

#include <QMutex>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QQueue>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QAtomicInt>

DFMBASE_BEGIN_NAMESPACE
class AbstractDirIterator;
DFMBASE_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

// 前向声明
class IteratorSearcherBridge;

class IteratorSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;
    friend class SimplifiedSearchWorker;

public:
    explicit IteratorSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);
    ~IteratorSearcher() override;

    // Required AbstractSearcher interface
    bool search() override;
    void stop() override;
    bool hasItem() const override;
    DFMSearchResultMap takeAll() override;
    QList<QUrl> takeAllUrls() override;

    // Static method to check if a URL is supported
    static bool isSupportSearch(const QUrl &url) { return true; }

signals:
    // 改为内部使用的信号
    void requestProcessNextDirectory();
    void requestCreateIterator(const QUrl &url);

public slots:
    // 处理迭代器创建回调
    void onIteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator);
    
    // 处理目录
    void processDirectory();

private:
    // 处理迭代器结果
    void processIteratorResults(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator);
    
    // 添加单个结果到结果映射
    void addResultToMap(const QUrl &fileUrl, DFMSearchResultMap &results);
    
    // 添加多个结果并发出信号
    void addResults(const DFMSearchResultMap &newResults);
    
    // 请求处理队列中的下一个目录
    void requestNextDirectory();

    // 发布批量结果
    void publishBatchedResults();

private:
    QAtomicInt status = kReady;
    DFMSearchResultMap resultMap;
    mutable QMutex mutex;
    QRegularExpression regex;
    QQueue<QUrl> pendingDirs;
    
    // 用于主线程与工作线程间通信的桥接对象
    QSharedPointer<IteratorSearcherBridge> bridge;

    // 批量处理相关
    QTimer *batchTimer;               // 批量定时器
    DFMSearchResultMap batchedResults; // 批量结果
    int batchResultLimit;             // 批量结果限制
    int batchTimeLimit;               // 批量时间限制(毫秒)
};

// 线程间安全通信的桥接类，总是在主线程中使用
class IteratorSearcherBridge : public QObject
{
    Q_OBJECT
public:
    explicit IteratorSearcherBridge(QObject *parent = nullptr);
    ~IteratorSearcherBridge() override;

    // 设置关联的搜索器指针
    void setSearcher(IteratorSearcher *searcher);
    
    // 请求创建迭代器 (在主线程调用)
    void createIterator(const QUrl &url);

signals:
    // 迭代器创建完成信号 (发送给工作线程)
    void iteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator);

private:
    // 使用QPointer跟踪搜索器生命周期
    QPointer<IteratorSearcher> searcherPtr;
};

DPSEARCH_END_NAMESPACE

#endif   // ITERATORSEARCHER_H
