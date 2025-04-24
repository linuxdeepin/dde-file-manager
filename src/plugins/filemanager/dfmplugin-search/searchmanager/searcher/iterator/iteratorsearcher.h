// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

DFMBASE_BEGIN_NAMESPACE
class AbstractDirIterator;
DFMBASE_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

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
    // Signal to request iterator creation in main thread
    void requestCreateIterator(const QUrl &url);

private slots:
    // Handler for search results processing
    void processDirectory(const QUrl &url);
    
    // Handler for iterator created in main thread
    void onIteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator, const QUrl &url);

private:
    QAtomicInt status = kReady;
    DFMSearchResultMap resultMap;
    mutable QMutex mutex;
    QRegularExpression regex;
    bool isStopped = false;
    QQueue<QUrl> pendingDirs;
    bool isProcessingQueue = false;
};

DPSEARCH_END_NAMESPACE

#endif   // ITERATORSEARCHER_H
