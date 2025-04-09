// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHINGSEARCH_H
#define ANYTHINGSEARCH_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <QSharedPointer>
#include <QElapsedTimer>
#include <QMutex>

class QDBusInterface;

DPSEARCH_BEGIN_NAMESPACE

class AnythingSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    explicit AnythingSearcher(const QUrl &url, const QString &keyword, bool isBindPath, QObject *parent = nullptr);
    virtual ~AnythingSearcher() override;

    static bool isSupported(const QUrl &url, bool &isBindPath);
    bool search() override;
    // void stop() override;
    bool hasItem() const override;
    QList<QUrl> takeAll() override;
    void tryNotify();

private:
    QDBusInterface *anythingInterface = nullptr;
    QAtomicInt status = kReady;
    QList<QUrl> allResults;
    mutable QMutex mutex;
    bool isBindPath;
    QString originalPath;

    //计时
    QElapsedTimer notifyTimer;
    qint64 lastEmit = 0;
};

DPSEARCH_END_NAMESPACE

#endif   // ANYTHINGSEARCH_H
