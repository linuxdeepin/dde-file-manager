// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"
#include "searchworker.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QThread>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent)
{
    qDebug() << "[Search] IteratorSearcher created with url:" << url.toString() << "keyword:" << key;
    
    // Create regex for search pattern
    regex = QRegularExpression(keyword, QRegularExpression::CaseInsensitiveOption);
    
    // Create worker and move to worker thread
    worker = QSharedPointer<SearchWorker>::create(url, regex);
    worker->moveToThread(&workerThread);
    
    // Connect worker signals to searcher slots
    connect(worker.data(), &SearchWorker::requestIteratorCreation, 
            this, &IteratorSearcher::createDirIterator);
    
    connect(worker.data(), &SearchWorker::resultsReady,
            this, &IteratorSearcher::onResultsReady);
    
    connect(worker.data(), &SearchWorker::searchFinished,
            this, &IteratorSearcher::onSearchFinished);
    
    // Connect thread finished signal to ensure proper cleanup
    connect(&workerThread, &QThread::finished, [this]() {
        if (worker) {
            worker.clear();
        }
    });
}

IteratorSearcher::~IteratorSearcher()
{
    // Ensure worker thread is properly terminated
    if (workerThread.isRunning()) {
        if (worker) {
            QMetaObject::invokeMethod(worker.data(), "requestStop", Qt::QueuedConnection);
        }
        
        workerThread.quit();
        if (!workerThread.wait(500)) {
            workerThread.terminate();
            workerThread.wait();
        }
    }
}

bool IteratorSearcher::search()
{
    // Atomic state transition from Ready to Running
    if (!status.testAndSetRelease(kReady, kRuning)) {
        qDebug() << "[Search] Cannot start search - not in Ready state";
        return false;
    }
    
    // Start worker thread and trigger search process
    workerThread.start();
    QMetaObject::invokeMethod(worker.data(), "startSearch", Qt::QueuedConnection);
    
    return true;
}

void IteratorSearcher::stop()
{
    // Mark as terminated
    status.storeRelease(kTerminated);
    
    // Ensure pending results are processed
    if (hasItem()) {
        emit unearthed(this);
    }
    
    // Request worker to stop
    if (worker && workerThread.isRunning()) {
        QMetaObject::invokeMethod(worker.data(), "requestStop", Qt::QueuedConnection);
    }
    
    // Terminate thread if running
    if (workerThread.isRunning()) {
        workerThread.quit();
        if (!workerThread.wait(200)) {
            workerThread.terminate();
            workerThread.wait();
        }
    }
    
    emit finished();
}

bool IteratorSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !resultMap.isEmpty();
}

DFMSearchResultMap IteratorSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(resultMap);
}

QList<QUrl> IteratorSearcher::takeAllUrls()
{
    QMutexLocker lk(&mutex);
    QList<QUrl> urls;
    urls.reserve(resultMap.size());
    
    for (auto it = resultMap.constBegin(); it != resultMap.constEnd(); ++it) {
        urls << it.key();
    }
    
    resultMap.clear();
    return urls;
}

void IteratorSearcher::createDirIterator(const QUrl &url)
{
    // This method is executed in main thread context
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    
    if (status.loadAcquire() != kRuning) {
        return;
    }
    
    // Create directory iterator
    auto iterator = DirIteratorFactory::create(url, QStringList(), 
                                              QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    if (!iterator) {
        return;
    }
    
    iterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::size,"
                         "standard::size,standard::is-symlink,standard::symlink-target,access::*,time::*");
    
    // Send iterator back to worker thread for processing
    QMetaObject::invokeMethod(worker.data(), "processDirIterator", 
                             Qt::QueuedConnection, 
                             Q_ARG(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator>, iterator),
                             Q_ARG(QUrl, url));
}

void IteratorSearcher::onResultsReady(const DFMSearchResultMap &results)
{
    if (status.loadAcquire() != kRuning) {
        return;
    }
    
    // Add results to result map without checking for duplicates
    {
        QMutexLocker lk(&mutex);
        for (auto it = results.constBegin(); it != results.constEnd(); ++it) {
            resultMap.insert(it.key(), it.value());
        }
    }
    
    // Notify about new results
    emit unearthed(this);
}

void IteratorSearcher::onSearchFinished()
{
    // Final state transition
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        if (hasItem()) {
            emit unearthed(this);
        }
        
        emit finished();
    }
}
