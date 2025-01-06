// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <QTime>
#include <QMutex>
#include <QRegularExpression>

DPSEARCH_BEGIN_NAMESPACE

class IteratorSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    explicit IteratorSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<QUrl> takeAll() override;
    void tryNotify();
    void doSearch();

private:
    QAtomicInt status = kReady;
    QList<QUrl> allResults;
    mutable QMutex mutex;
    QList<QUrl> searchPathList;
    QRegularExpression regex;

    //计时
    QElapsedTimer notifyTimer;
    int lastEmit = 0;
};

DPSEARCH_END_NAMESPACE

#endif   // ITERATORSEARCHER_H
