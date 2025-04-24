// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QApplication>
#include <QMetaObject>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent),
      isStopped(false),
      isProcessingQueue(false)
{
    // Create regex for search pattern
    regex = QRegularExpression(keyword);
}

IteratorSearcher::~IteratorSearcher()
{
    pendingDirs.clear();
}

bool IteratorSearcher::search()
{
    // Atomic state transition from Ready to Running
    if (!status.testAndSetRelease(kReady, kRuning)) {
        return false;
    }

    // Connect our signal to the main thread's slot using Qt::ConnectionType::BlockingQueuedConnection
    // This must be set up before search starts
    QObject *mainThreadReceiver = qApp;
    connect(this, &IteratorSearcher::requestCreateIterator, mainThreadReceiver, 
            [this](const QUrl &url) {
                // This lambda runs in the main thread
                if (status.loadAcquire() != kRuning || isStopped) {
                    return;
                }
                
                // Create iterator in main thread
                auto iterator = DirIteratorFactory::create(url, QStringList(), 
                                                         QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
                if (!iterator) {
                    // Signal back to our worker thread
                    QMetaObject::invokeMethod(this, "onIteratorCreated", Qt::QueuedConnection,
                                           Q_ARG(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator>, nullptr),
                                           Q_ARG(QUrl, url));
                    return;
                }
                
                iterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::size,"
                                    "standard::is-symlink,standard::symlink-target,access::*,time::*");
                
                // Pass the iterator back to our worker thread
                QMetaObject::invokeMethod(this, "onIteratorCreated", Qt::QueuedConnection,
                                       Q_ARG(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator>, iterator),
                                       Q_ARG(QUrl, url));
            }, 
            Qt::ConnectionType::QueuedConnection);

    // Start with the root search URL
    processDirectory(searchUrl);
    
    return true;
}

void IteratorSearcher::stop()
{
    // Mark as terminated
    status.storeRelease(kTerminated);
    isStopped = true;
    
    // Clear pending directories
    pendingDirs.clear();
    
    // Ensure pending results are processed
    if (hasItem())
        emit unearthed(this);
    
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
    DFMSearchResultMap results = resultMap;
    resultMap.clear();
    return results;
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

void IteratorSearcher::processDirectory(const QUrl &url)
{
    if (status.loadAcquire() != kRuning || isStopped)
        return;

    // Add to pending queue
    if (url.isValid())
        pendingDirs.enqueue(url);

    // If all done, notify finished
    if (pendingDirs.isEmpty() && !isStopped && status.loadAcquire() == kRuning) {
        status.storeRelease(kCompleted);
        emit finished();
    }

    // If already processing queue, just return
    if (isProcessingQueue)
        return;
    
    // Process the first URL in the queue
    isProcessingQueue = true;
    
    // Process all pending directories
    while (!pendingDirs.isEmpty() && !isStopped && status.loadAcquire() == kRuning) {
        QUrl currentUrl = pendingDirs.dequeue();
        
        // Request iterator creation in the main thread
        emit requestCreateIterator(currentUrl);
    }
    
    isProcessingQueue = false;
}

void IteratorSearcher::onIteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator, const QUrl &url)
{
    if (status.loadAcquire() != kRuning || isStopped)
        return;
    
    // If iterator creation failed, just move on
    if (!iterator) {
        // Continue processing the queue
        QMetaObject::invokeMethod(this, "processDirectory", Qt::QueuedConnection, 
                                 Q_ARG(QUrl, QUrl()));
        return;
    }
    
    // Process directory entries with our iterator
    DFMSearchResultMap newResults;
    QList<QUrl> subDirs;
    
    while (iterator->hasNext() && !isStopped) {
        if (status.loadAcquire() != kRuning) {
            break;
        }
        
        QUrl fileUrl = iterator->next();
        auto info = iterator->fileInfo();
        if (!info || !info->exists())
            continue;
        
        // Add subdirectories to search queue
        if (info->isAttributes(OptInfoType::kIsDir) && !info->isAttributes(OptInfoType::kIsSymLink)) {
            const auto &dirUrl = info->urlOf(UrlInfoType::kUrl);
            if (!dirUrl.path().startsWith("/sys/")) {
                subDirs << dirUrl;
            }
        }
        
        // Check if file name matches search pattern
        if (regex.match(info->displayOf(DisPlayInfoType::kFileDisplayName)).hasMatch()) {
            // Create search result
            DFMSearchResult result;
            result.setUrl(fileUrl);
            result.setMatchScore(1.0);  // Default match score
            
            // Add to results
            newResults.insert(fileUrl, result);
        }
    }

    // Process results
    if (!newResults.isEmpty() && !isStopped) {
        // Add results to result map
        {
            QMutexLocker lk(&mutex);
            for (auto it = newResults.constBegin(); it != newResults.constEnd(); ++it) {
                resultMap.insert(it.key(), it.value());
            }
        }
        
        // Notify about new results
        emit unearthed(this);
    }
    
    // Add subdirectories to the queue
    for (const QUrl &subDir : subDirs) {
        pendingDirs.enqueue(subDir);
    }
    
    // Continue processing the queue
    QMetaObject::invokeMethod(this, "processDirectory", Qt::QueuedConnection, 
                             Q_ARG(QUrl, QUrl()));
}
