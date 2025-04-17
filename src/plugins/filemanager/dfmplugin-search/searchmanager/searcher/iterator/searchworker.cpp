// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchworker.h"
#include "utils/searchhelper.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QCoreApplication>

static constexpr char kFilterFolders[] = "^/(dev|proc|sys|run|tmpfs).*$";

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

SearchWorker::SearchWorker(const QUrl &rootUrl, const QRegularExpression &searchPattern, QObject *parent)
    : QObject(parent)
    , rootUrl(rootUrl)
    , searchPattern(searchPattern)
    , status(Ready)
    , activeRequests(0)
{
    pendingUrls.enqueue(rootUrl);
    emitInterval = 50; // 50ms between result updates
    
    // Start timer for tracking result emission
    resultTimer.start();
    
    qDebug() << "[Search] SearchWorker created for URL:" << rootUrl.toString();
}

SearchWorker::~SearchWorker()
{
    // Ensure we're properly cleaned up
    if (status.loadAcquire() == Running) {
        requestStop();
    }
}

void SearchWorker::startSearch()
{
    // Ensure we're in the Ready state before starting
    if (!status.testAndSetRelease(Ready, Running)) {
        qDebug() << "[Search] Cannot start search - not in Ready state";
        return;
    }
    
    // Reset timer and start processing
    resultTimer.restart();
    lastEmitTime = 0;
    
    // Begin processing URLs in the queue
    QMetaObject::invokeMethod(this, "processNextUrl", Qt::QueuedConnection);
}

void SearchWorker::requestStop()
{
    // Mark as terminated
    status.storeRelease(Terminated);
    
    // Emit any remaining results
    emitPendingResults();
    
    // Notify search completion
    emit searchFinished();
}

void SearchWorker::processNextUrl()
{
    // Get next URL to process, if any
    QUrl url;
    {
        QMutexLocker locker(&urlMutex);
        if (!pendingUrls.isEmpty() && status.loadAcquire() == Running) {
            url = pendingUrls.dequeue();
        }
    }
    
    if (!url.isEmpty()) {
        // Increment active request counter
        activeRequests.fetchAndAddRelease(1);
        
        // Request iterator creation in main thread
        emit requestIteratorCreation(url);
    } else if (activeRequests.loadAcquire() == 0 && status.loadAcquire() == Running) {
        // No more URLs and no active requests, search is complete
        status.storeRelease(Completed);
        emitPendingResults();
        emit searchFinished();
    }
}

void SearchWorker::processDirIterator(QSharedPointer<AbstractDirIterator> iterator, const QUrl &url)
{
    // Check if we should still be running
    if (status.loadAcquire() != Running || !iterator) {
        activeRequests.fetchAndAddRelease(-1);
        QMetaObject::invokeMethod(this, "processNextUrl", Qt::QueuedConnection);
        return;
    }

    // Apply directory filters
    QRegularExpression reg(kFilterFolders);
    const auto &searchRootPath = rootUrl.toLocalFile();
    const auto &filePath = url.toLocalFile();
    
    if (url.isLocalFile() && !reg.match(searchRootPath).hasMatch() && reg.match(filePath).hasMatch()) {
        activeRequests.fetchAndAddRelease(-1);
        QMetaObject::invokeMethod(this, "processNextUrl", Qt::QueuedConnection);
        return;
    }

    DFMSearchResultMap tempResults;
    QList<QUrl> newDirs;
    
    // Process directory contents
    while (iterator->hasNext() && status.loadAcquire() == Running) {
        iterator->next();
        auto info = iterator->fileInfo();
        if (!info || !info->exists())
            continue;
        
        // Add subdirectories to search queue
        if (info->isAttributes(OptInfoType::kIsDir) && !info->isAttributes(OptInfoType::kIsSymLink)) {
            const auto &dirUrl = info->urlOf(UrlInfoType::kUrl);
            if (!dirUrl.path().startsWith("/sys/")) {
                newDirs << dirUrl;
            }
        }

        // Check if file name matches search pattern
        QRegularExpressionMatch match = searchPattern.match(info->displayOf(DisPlayInfoType::kFileDisplayName));
        if (match.hasMatch()) {
            const auto &fileUrl = info->urlOf(UrlInfoType::kUrl);
            info->updateAttributes();
            
            DFMSearchResult result(fileUrl);
            result.setMatchScore(1.0);
            result.setIsContentMatch(false);
            tempResults.insert(fileUrl, result);
        }
    }
    
    // Update URL queue with new directories
    if (!newDirs.isEmpty()) {
        QMutexLocker locker(&urlMutex);
        for (const auto &dirUrl : newDirs) {
            if (!pendingUrls.contains(dirUrl)) {
                pendingUrls.enqueue(dirUrl);
            }
        }
    }
    
    // Add found results to the pending list
    if (!tempResults.isEmpty()) {
        QMutexLocker locker(&resultMutex);
        for (auto it = tempResults.constBegin(); it != tempResults.constEnd(); ++it) {
            if (!pendingResults.contains(it.key())) {
                pendingResults.insert(it.key(), it.value());
            }
        }
        
        // Check if we should emit results
        int elapsed = resultTimer.elapsed();
        if ((elapsed - lastEmitTime) > emitInterval) {
            QMetaObject::invokeMethod(this, "emitPendingResults", Qt::QueuedConnection);
        }
    }
    
    // Decrement active request counter and continue to next URL
    activeRequests.fetchAndAddRelease(-1);
    QMetaObject::invokeMethod(this, "processNextUrl", Qt::QueuedConnection);
}

void SearchWorker::emitPendingResults()
{
    DFMSearchResultMap results;
    
    {
        QMutexLocker locker(&resultMutex);
        if (pendingResults.isEmpty())
            return;
            
        results = pendingResults;
        pendingResults.clear();
    }
    
    // Update last emit time
    lastEmitTime = resultTimer.elapsed();
    
    // Emit results
    if (!results.isEmpty()) {
        emit resultsReady(results);
    }
} 
