// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_P_H
#define FULLTEXTSEARCHER_P_H

#include "searchmanager/searcher/abstractsearcher.h"

#include <lucene++/LuceneHeaders.h>

#include <QStandardPaths>
#include <QApplication>
#include <QMutex>
#include <QTime>
#include <QAtomicInt>
#include <QWaitCondition>

DPSEARCH_BEGIN_NAMESPACE

class FullTextSearcher;
class FullTextSearcherPrivate : public QObject
{
    Q_OBJECT
    friend class FullTextSearcher;

public:
    enum WordType {
        kCn,
        kEn,
        kDigit,
        kSymbol
    };

    enum TaskType {
        kCreate,
        kUpdate
    };

    enum IndexType {
        kAddIndex,
        kUpdateIndex,
        kDeleteIndex
    };
    Q_ENUM(IndexType)

    explicit FullTextSearcherPrivate(FullTextSearcher *parent);
    ~FullTextSearcherPrivate();

private:
    Lucene::IndexWriterPtr newIndexWriter(bool create = false);
    Lucene::IndexReaderPtr newIndexReader();

    bool doSearch(const QString &path, const QString &keyword);
    inline static QString indexStorePath()
    {
        static QString path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
                + "/deepin/dde-file-manager/index";
        return path;
    }

    QString dealKeyword(const QString &keyword);
    void tryNotify();

    bool isUpdated = false;
    QAtomicInt status = AbstractSearcher::kReady;
    QAtomicInt taskStatus { 0 };
    QMutex taskMutex;
    QWaitCondition taskCondition;
    QList<QUrl> allResults;
    mutable QMutex mutex;
    static bool isIndexCreating;
    QMap<QString, QString> bindPathTable;

    //计时
    QTime notifyTimer;
    int lastEmit = 0;

    FullTextSearcher *q = nullptr;

    void doSearchAndEmit(const QString &path, const QString &key);

    QSet<QString> invalidIndexPaths;   // 存储无效的索引路径
};

DPSEARCH_END_NAMESPACE

#endif   // FULLTEXTSEARCHER_P_H
