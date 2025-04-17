// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include "dfm-base/dfm_base_global.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchworker.h"

#include <QMutex>
#include <QRegularExpression>
#include <QThread>
#include <QSharedPointer>

DFMBASE_BEGIN_NAMESPACE
class AbstractDirIterator;
DFMBASE_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

class IteratorSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    explicit IteratorSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);
    ~IteratorSearcher() override;

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    DFMSearchResultMap takeAll() override;
    QList<QUrl> takeAllUrls() override;

private slots:
    // Handler for directory iterator creation in main thread
    void createDirIterator(const QUrl &url);
    
    // Handler for receiving search results from worker
    void onResultsReady(const DFMSearchResultMap &results);
    
    // Handler for search completion
    void onSearchFinished();

private:
    QAtomicInt status = kReady;
    DFMSearchResultMap resultMap;
    mutable QMutex mutex;
    QRegularExpression regex;
    QThread workerThread;
    QSharedPointer<SearchWorker> worker;
};

DPSEARCH_END_NAMESPACE

#endif   // ITERATORSEARCHER_H
