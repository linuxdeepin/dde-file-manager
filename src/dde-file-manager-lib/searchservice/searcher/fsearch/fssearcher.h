// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSSEARCHER_H
#define FSSEARCHER_H

#include "abstractsearcher.h"
#include "durl.h"

extern "C" {
#include "fsearch/fsearch.h"
}

#include <QTime>
#include <QMutex>
#include <QWaitCondition>

class FsSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    explicit FsSearcher(const DUrl &url, const QString &key, QObject *parent = nullptr);
    ~FsSearcher() override;

    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<DUrl> takeAll() override;

    void initApp();
    void tryNotify();
    static bool isSupported(const DUrl &url);
    static void cbReceiveResults(void *data, void *sender);

private:
    FsearchApplication *app = nullptr;
    QAtomicInt status = kReady;
    bool isWorking = false;
    //搜索结果
    mutable QMutex mutex;
    QWaitCondition waitCondition;
    QMutex conditionMtx;
    QList<DUrl> allResults;
    QHash<QString, QSet<QString>> hiddenFilters;

    //搜索计时
    QTime notifyTimer;
    int lastEmit = 0;
};

#endif // FSSEARCHER_H
