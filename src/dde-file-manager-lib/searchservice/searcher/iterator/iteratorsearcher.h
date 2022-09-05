// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include "abstractsearcher.h"

#include <QTime>
#include <QMutex>
#include <QRegularExpression>

class IteratorSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    explicit IteratorSearcher(const DUrl &url, const QString &key, QObject *parent = nullptr);

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<DUrl> takeAll() override;
    void tryNotify();
    void doSearch();

private:
    QAtomicInt status = kReady;
    QList<DUrl> allResults;
    mutable QMutex mutex;
    QList<DUrl> searchPathList;
    QRegularExpression regex;

    //计时
    QTime notifyTimer;
    int lastEmit = 0;
};

#endif   // ITERATORSEARCHER_H
