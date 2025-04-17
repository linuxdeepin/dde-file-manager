// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include "dfmplugin_search_global.h"

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QUrl>
#include <QMutex>
#include <QQueue>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QAtomicInt>

DFMBASE_BEGIN_NAMESPACE
class AbstractDirIterator;
DFMBASE_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

class DFMSearchResult;
using DFMSearchResultMap = QMap<QUrl, DFMSearchResult>;

// SearchWorker encapsulates the actual search process
// It's designed to run in a worker thread and communicate
// via signals/slots with the main thread
class SearchWorker : public QObject
{
    Q_OBJECT

public:
    enum WorkerStatus {
        Ready,
        Running,
        Completed,
        Terminated
    };

    explicit SearchWorker(const QUrl &rootUrl, const QRegularExpression &searchPattern, QObject *parent = nullptr);
    ~SearchWorker() override;

public slots:
    // Entry point to start the search process
    void startSearch();
    
    // Process directory iterator created in main thread
    void processDirIterator(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator, const QUrl &url);
    
    // Request to stop the search process
    void requestStop();

signals:
    // Signal to request creation of directory iterator in main thread
    void requestIteratorCreation(const QUrl &url);
    
    // Signal for new search results
    void resultsReady(const DFMSearchResultMap &results);
    
    // Signal for search completion
    void searchFinished();
    
private slots:
    // Process next URL in the queue
    void processNextUrl();
    
    // Notify about found results periodically
    void emitPendingResults();

private:
    // Root URL to start the search from
    QUrl rootUrl;
    
    // Regular expression pattern to match
    QRegularExpression searchPattern;
    
    // Queue of URLs to process
    QQueue<QUrl> pendingUrls;
    
    // Mutex to protect URL queue
    QMutex urlMutex;
    
    // Temporary buffer for search results
    DFMSearchResultMap pendingResults;
    
    // Mutex to protect results
    QMutex resultMutex;
    
    // Timer for emitting results periodically
    QElapsedTimer resultTimer;
    
    // Last time results were emitted
    int lastEmitTime = 0;
    
    // Emit interval in milliseconds
    int emitInterval = 50;
    
    // Atomic status flag
    QAtomicInt status;
    
    // Active request count for pending iterators
    QAtomicInt activeRequests;
};

DPSEARCH_END_NAMESPACE

#endif // SEARCHWORKER_H 
