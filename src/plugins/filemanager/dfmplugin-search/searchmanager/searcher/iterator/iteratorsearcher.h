// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include "dfm-base/dfm_base_global.h"
#include "searchmanager/searcher/abstractsearcher.h"

#include <QTime>
#include <QMutex>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QThread>
#include <QWaitCondition>
#include <QQueue>
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
    void tryNotify();

private slots:
    void doSearch();
    void createDirIterator(const QUrl &url);
    void processDirIterator(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator, const QUrl &url);
    void onSearchThreadFinished();

signals:
    void requestIteratorCreation(const QUrl &url);
    void iteratorCreated(QSharedPointer<DFMBASE_NAMESPACE::AbstractDirIterator> iterator, const QUrl &url);

private:
    QAtomicInt status = kReady;
    DFMSearchResultMap resultMap;
    mutable QMutex mutex;
    QList<QUrl> searchPathList;
    QRegularExpression regex;
    QThread searchThread;
    QMutex pathMutex;
    QWaitCondition iteratorCondition;

    //计时
    QElapsedTimer notifyTimer;
    int lastEmit = 0;
};

DPSEARCH_END_NAMESPACE

#endif   // ITERATORSEARCHER_H
