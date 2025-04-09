// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEARCHER_H
#define FSEARCHER_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>

DPSEARCH_BEGIN_NAMESPACE

class FSearchHandler;
class FSearcher : public AbstractSearcher
{
    Q_OBJECT
public:
    explicit FSearcher(const QUrl &url, const QString &key, QObject *parent = nullptr);
    ~FSearcher() override;

    static bool isSupport(const QUrl &url);
    bool search() override;
    // void stop() override;
    bool hasItem() const override;
    QList<QUrl> takeAll() override;
    void tryNotify();

private:
    static void receiveResultCallback(const QString &result, bool isFinished, FSearcher *self);

private:
    FSearchHandler *searchHandler = nullptr;
    QAtomicInt status = kReady;
    QList<QUrl> allResults;
    mutable QMutex mutex;
    QWaitCondition waitCondition;
    QMutex conditionMtx;
    QHash<QString, QSet<QString>> hiddenFileHash;

    //计时
    QElapsedTimer notifyTimer;
    qint64 lastEmit = 0;
};

DPSEARCH_END_NAMESPACE

#endif // FSEARCHER_H
