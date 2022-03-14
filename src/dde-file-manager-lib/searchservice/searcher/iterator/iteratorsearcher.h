/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
