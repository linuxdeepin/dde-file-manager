// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iteratorsearcher.h"
#include "utils/searchhelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QThread>

static int kEmitInterval = 50;   // 推送时间间隔（ms
static constexpr char kFilterFolders[] = "^/(dev|proc|sys|run|tmpfs).*$";

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

IteratorSearcher::IteratorSearcher(const QUrl &url, const QString &key, QObject *parent)
    : AbstractSearcher(url, SearchHelper::instance()->checkWildcardAndToRegularExpression(key), parent)
{
    searchPathList << url;
    regex = QRegularExpression(keyword, QRegularExpression::CaseInsensitiveOption);
    
    qDebug() << "[Search] IteratorSearcher created with url:" << url.toString() << "keyword:" << key;
    
    // Connect the iterator creation signal to the processing slot with queued connection
    // This allows safe thread transition for directory iterators from main to worker thread
    connect(this, &IteratorSearcher::iteratorCreated, 
            this, &IteratorSearcher::processDirIterator, 
            Qt::QueuedConnection);
    
    // Connect the request signal to main thread handler
    // This connection is crucial for thread transitions (worker→main)
    connect(this, &IteratorSearcher::requestIteratorCreation,
            QCoreApplication::instance(), [this](const QUrl &url) {
                this->createDirIterator(url);
            }, Qt::QueuedConnection);
    
    // Connect thread finished signal to cleanup handler
    // IMPORTANT: Uses direct connection as QThread::finished is emitted from the thread
    // that calls quit() or exit(), not from the thread itself
    connect(&searchThread, &QThread::finished, 
            this, &IteratorSearcher::onSearchThreadFinished, 
            Qt::DirectConnection);
    
    // Move searcher object to worker thread
    this->moveToThread(&searchThread);
}

IteratorSearcher::~IteratorSearcher()
{
    // Ensure thread is properly terminated
    if (searchThread.isRunning()) {
        searchThread.quit();
        if (!searchThread.wait(500)) {
            searchThread.terminate();
            searchThread.wait();
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

    notifyTimer.start();
    
    // Start worker thread and trigger search process
    searchThread.start();
    QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
    
    return true;
}

void IteratorSearcher::stop()
{
    // Mark as terminated and ensure pending results are processed
    status.storeRelease(kTerminated);
    
    if (hasItem()) {
        emit unearthed(this);
    }
    
    // Terminate thread if running
    if (searchThread.isRunning()) {
        searchThread.quit();
        if (!searchThread.wait(200)) {
            searchThread.terminate();
            searchThread.wait();
        }
        
        // Ensure cleanup handler is called
        if (status.loadAcquire() == kTerminated) {
            onSearchThreadFinished();
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

void IteratorSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > kEmitInterval) {
        lastEmit = cur;
        emit unearthed(this);
    }
}

void IteratorSearcher::doSearch()
{
    // Get next URL to process
    QUrl url;
    {
        QMutexLocker locker(&pathMutex);
        if (!searchPathList.isEmpty() && status.loadAcquire() == kRuning) {
            url = searchPathList.takeAt(0);
        }
    }
    
    if (!url.isEmpty()) {
        // Create directory iterator in main thread using a signal/slot based approach
        emit requestIteratorCreation(url);
    } else if (status.loadAcquire() == kRuning) {
        // Search complete, all URLs processed
        qDebug() << "[Search] All directories processed, quitting search thread";
        searchThread.quit();
    }
}

// Handler for creating directory iterators in main thread context
void IteratorSearcher::createDirIterator(const QUrl &url)
{
    // This method must be connected to requestIteratorCreation signal 
    // and executed in main thread context
    
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    
    if (status.loadAcquire() != kRuning) {
        QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
        return;
    }
    
    // Apply directory filters
    QRegularExpression reg(kFilterFolders);
    const auto &searchRootPath = searchUrl.toLocalFile();
    const auto &filePath = url.toLocalFile();
    
    if (url.isLocalFile() && !reg.match(searchRootPath).hasMatch() && reg.match(filePath).hasMatch()) {
        QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
        return;
    }
    
    // Create directory iterator
    auto iterator = DirIteratorFactory::create(url, QStringList(), 
                                              QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    if (!iterator) {
        QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
        return;
    }
    
    iterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::size,"
                         "standard::size,standard::is-symlink,standard::symlink-target,access::*,time::*");
    
    // Send iterator back to worker thread
    emit iteratorCreated(iterator, url);
}

void IteratorSearcher::processDirIterator(QSharedPointer<AbstractDirIterator> iterator, const QUrl &url)
{
    // Process each item from the iterator
    if (status.loadAcquire() != kRuning || !iterator) {
        QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
        return;
    }

    int itemCount = 0;
    int matchCount = 0;
    int dirCount = 0;
    
    // Process directory contents
    while (iterator->hasNext() && status.loadAcquire() == kRuning) {
        iterator->next();
        auto info = iterator->fileInfo();
        if (!info || !info->exists())
            continue;

        itemCount++;
        
        // Add subdirectories to search queue
        if (info->isAttributes(OptInfoType::kIsDir) && !info->isAttributes(OptInfoType::kIsSymLink)) {
            const auto &dirUrl = info->urlOf(UrlInfoType::kUrl);
            if (!dirUrl.path().startsWith("/sys/")) {
                QMutexLocker locker(&pathMutex);
                if (!searchPathList.contains(dirUrl)) {
                    searchPathList << dirUrl;
                    dirCount++;
                }
            }
        }

        // Check if file name matches search pattern
        QRegularExpressionMatch match = regex.match(info->displayOf(DisPlayInfoType::kFileDisplayName));
        if (match.hasMatch()) {
            const auto &fileUrl = info->urlOf(UrlInfoType::kUrl);
            {
                info->updateAttributes();
                QMutexLocker lk(&mutex);
                
                if (!resultMap.contains(fileUrl)) {
                    DFMSearchResult result(fileUrl);
                    result.setMatchScore(1.0);
                    result.setIsContentMatch(false);
                    resultMap.insert(fileUrl, result);
                    matchCount++;
                }
            }

            tryNotify();
        }
    }
    
    // Cleanup and continue to next URL
    iterator.clear();
    QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
}

void IteratorSearcher::onSearchThreadFinished()
{
    // Final state transition and cleanup
    if (status.testAndSetRelease(kRuning, kCompleted)) {
        if (hasItem()) {
            emit unearthed(this);
        }
        
        emit finished();
    }
}
