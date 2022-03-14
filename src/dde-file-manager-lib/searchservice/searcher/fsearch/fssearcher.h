/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
